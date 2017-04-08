#ifndef INCLUDED_ISHADER_H
#define INCLUDED_ISHADER_H
#include "core/lxString.h"
#include "math/matrix4.h"
#include "video/Color.h"
#include "BaseTexture.h"
#include "MaterialRenderer.h"

namespace lux
{
namespace io
{
class File;
}
namespace video
{

class Shader : public ReferenceCounted
{
	friend class ShaderParam;
private:
	virtual void GetShaderValue(u32 RegisterVS, u32 RegisterPS, core::Type type, u32 Size, void* out) = 0;
	virtual void SetShaderValue(u32 RegisterVS, u32 RegisterPS, core::Type type, u32 Size, const void* data) = 0;

public:
	virtual ~Shader()
	{
	}

	virtual const ShaderParam& GetParam(const char* pcName) = 0;
	virtual const ShaderParam& GetParam(u32 index) = 0;
	virtual u32 GetParamCount() const = 0;
	virtual void Enable() = 0;
	virtual void LoadParams(const core::PackagePuffer& Puffer) = 0;
	virtual void Disable() = 0;

	virtual bool Init(const char* pcVSCode, const char* pcVSEntryPoint, size_t VSLength, const char* pcVSProfile,
		const char* pcPSCode, const char* pcPSEntryPoint, size_t PSLength, const char* pcPSProfile) = 0;

	virtual const core::ParamPackage& GetParamPackage() = 0;
};

class LUX_API ShaderParam
{
	friend class ShaderValueAccess;
private:
	class ShaderValueAccess
	{
		friend class ShaderParam;
	private:
		const ShaderParam* Param;
		ShaderValueAccess(const ShaderParam* p) : Param(p)
		{
		}
	public:
		float Matrix(int iRow, int iCol);            // Matrix only

		// Getter
		operator bool();
		operator int();
		operator float();
		operator math::vector2f();
		operator math::vector3f();
		operator video::Colorf();
		operator math::matrix4();

		// Setter
		ShaderValueAccess& operator= (const ShaderValueAccess& varVal);
		ShaderValueAccess& operator= (bool bVal);
		ShaderValueAccess& operator= (int iVal);
		ShaderValueAccess& operator= (float fVal);
		ShaderValueAccess& operator= (const math::vector2f& vVal);
		ShaderValueAccess& operator= (const math::vector3f& vVal);
		ShaderValueAccess& operator= (const video::Colorf& Val);
		ShaderValueAccess& operator= (const math::matrix4& mVal);
	};

private:
	u32            m_Register_VS;    // Das Register im Vertexshader
	u32            m_Register_PS;    // Das Register im Pixelshader

// The type core::Type is independet of the client or dll.
#pragma warning(suppress: 4251)
	core::Type    m_Type;            // Der Datentyp
	u8            m_TypeSize;        // Die größe des Datentyps
	const char* m_pName;        // Der name dieses Members

	Shader*    m_pOwner;        // Der zugehörige Shader

public:
	// Konstruktor/Destruktor
	ShaderParam(Shader *pOwnerEffect,
		core::Type type, u8 TypeSize,
		const char* const pName,
		u32 RegisterVS, u32 RegisterPS) : m_pOwner(pOwnerEffect),
		m_Type(type), m_TypeSize(TypeSize),
		m_Register_VS(RegisterVS), m_Register_PS(RegisterPS),
		m_pName(pName)
	{
	}

	ShaderParam() : m_pOwner(nullptr),
		m_Type(core::Type::Unknown), m_TypeSize(0),
		m_Register_VS(0xFFFFFFFF), m_Register_PS(0xFFFFFFFF),
		m_pName(nullptr)
	{
	}

	ShaderParam(const ShaderParam& other) : m_Register_PS(other.m_Register_PS),
		m_Register_VS(other.m_Register_VS),
		m_Type(other.m_Type), m_TypeSize(other.m_TypeSize),
		m_pName(other.m_pName), m_pOwner(other.m_pOwner)
	{
	}

	bool IsValid() const
	{
		return (m_Register_PS != -1) || (m_Register_VS != -1);
	}

	core::Type GetType() const
	{
		return m_Type;
	}

	const char* const GetName() const
	{
		return m_pName;
	}

	ShaderValueAccess operator*() const
	{
		return ShaderValueAccess(this);
	}

	ShaderParam& operator= (const ShaderParam& varVal);

	void GetShaderValue(void* out) const
	{
		m_pOwner->GetShaderValue(m_Register_VS, m_Register_PS, m_Type, m_TypeSize, out);
	}
	void SetShaderValue(const void* data) const
	{
		m_pOwner->SetShaderValue(m_Register_VS, m_Register_PS, m_Type, m_TypeSize, data);
	}
};


inline ShaderParam::ShaderValueAccess& ShaderParam::ShaderValueAccess::operator= (const ShaderParam::ShaderValueAccess& Value)
{
	if(Param->IsValid() && Value.Param->IsValid() &&
		Param->m_pOwner == Value.Param->m_pOwner &&
		Param->m_Type == Value.Param->m_Type &&
		Param->m_Type != core::Type::Unknown) {
		static u8 data[64];

		Value.Param->GetShaderValue(data);
		Param->SetShaderValue(data);
	}

	return *this;
}

inline ShaderParam::ShaderValueAccess& ShaderParam::ShaderValueAccess::operator= (bool Value)
{
	Param->SetShaderValue(&Value);
	return *this;
}

inline ShaderParam::ShaderValueAccess& ShaderParam::ShaderValueAccess::operator= (int Value)
{
	Param->SetShaderValue(&Value);
	return *this;
}

inline ShaderParam::ShaderValueAccess& ShaderParam::ShaderValueAccess::operator= (float Value)
{
	Param->SetShaderValue(&Value);
	return *this;
}

inline ShaderParam::ShaderValueAccess& ShaderParam::ShaderValueAccess::operator= (const math::vector2f& Value)
{
	float a[2] = {Value.x, Value.y};
	Param->SetShaderValue(a);
	return *this;
}

inline ShaderParam::ShaderValueAccess& ShaderParam::ShaderValueAccess::operator= (const math::vector3f& Value)
{
	float a[3] = {Value.x, Value.y, Value.z};
	Param->SetShaderValue(a);
	return *this;
}

inline ShaderParam::ShaderValueAccess& ShaderParam::ShaderValueAccess::operator= (const video::Colorf& Value)
{
	float a[4] = {Value.r, Value.g, Value.b, Value.a};
	Param->SetShaderValue(a);
	return *this;
}

inline ShaderParam::ShaderValueAccess& ShaderParam::ShaderValueAccess::operator= (const math::matrix4& Value)
{
	Param->SetShaderValue(Value.DataRowMajor());
	return *this;
}

//Liefert einen Wert aus dem Effekt
inline ShaderParam::ShaderValueAccess::operator bool()
{
	bool out;
	Param->GetShaderValue(&out);

	return out;
}

//Liefert einen Wert aus dem Effekt
inline ShaderParam::ShaderValueAccess::operator int()
{
	int out;
	Param->GetShaderValue(&out);

	return out;
}

//Liefert einen Wert aus dem Effekt
inline ShaderParam::ShaderValueAccess::operator float()
{
	float out;
	Param->GetShaderValue(&out);

	return out;
}

//Liefert einen Wert aus dem Effekt
inline ShaderParam::ShaderValueAccess::operator math::vector2f()
{
	float a[2];
	Param->GetShaderValue(a);

	return math::vector2f(a[0], a[1]);
}

//Liefert einen Wert aus dem Effekt
inline ShaderParam::ShaderValueAccess::operator math::vector3f()
{
	float a[3];
	Param->GetShaderValue(a);

	return math::vector3f(a[0], a[1], a[2]);
}

//Liefert einen Wert aus dem Effekt
inline ShaderParam::ShaderValueAccess::operator video::Colorf()
{
	video::Colorf out;
	float a[4];
	Param->GetShaderValue(a);

	return video::Colorf(a[0], a[1], a[2], a[3]);
}

//Liefert einen Wert aus dem Effekt
inline ShaderParam::ShaderValueAccess::operator math::matrix4()
{
	math::matrix4 out;
	Param->GetShaderValue(out.DataRowMajor());

	return out;
}

inline float ShaderParam::ShaderValueAccess::Matrix(int iRow, int iCol)
{
	return this->operator lux::math::matrix4()(iRow, iCol);
}

}    // namespace video
}    // namespace lux
#endif
