#ifndef INCLUDED_SHADER_H
#define INCLUDED_SHADER_H
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

struct ShaderCompileException : public core::Exception
{
	explicit ShaderCompileException(const char* msg = "shader compile error") :
		Exception(msg)
	{}
};

class ShaderParam;

class Shader : public ReferenceCounted
{
	friend class ShaderParam;
public:
	virtual ~Shader() {}

	virtual const ShaderParam& GetParam(const char* name) = 0;
	virtual const ShaderParam& GetParam(u32 index) = 0;
	virtual u32 GetParamCount() const = 0;
	virtual void Enable() = 0;
	virtual void LoadParams(const core::PackagePuffer& puffer, const RenderData* renderData) = 0;
	virtual void LoadSceneValues() = 0;
	virtual void Disable() = 0;

	virtual void Init(
		const char* vsCode, const char* vsEntryPoint, size_t vsLength, const char* vsProfile,
		const char* psCode, const char* psEntryPoint, size_t psLength, const char* psProfile,
		core::array<string>* errorList) = 0;

	virtual core::ParamPackage& GetParamPackage() = 0;

private:
	virtual void GetShaderValue(
		u32 registerVS, u32 registerPS,
		u32 registerVSCount, u32 registerPSCount,
		core::Type type, u32 size, void* out) = 0;
	virtual void SetShaderValue(
		u32 registerVS, u32 registerPS,
		u32 registerVSCount, u32 registerPSCount,
		core::Type type, u32 size, const void* data) = 0;
};

class ShaderParam
{
	friend class ShaderValueAccess;
private:
	class ShaderValueAccess
	{
		friend class ShaderParam;
	public:
		float Matrix(int row, int col);

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
		ShaderValueAccess& operator= (bool value);
		ShaderValueAccess& operator= (int value);
		ShaderValueAccess& operator= (float value);
		ShaderValueAccess& operator= (const math::vector2f& value);
		ShaderValueAccess& operator= (const math::vector3f& value);
		ShaderValueAccess& operator= (const video::Colorf& value);
		ShaderValueAccess& operator= (const math::matrix4& value);

	private:
		const ShaderParam* m_Param;
		ShaderValueAccess(const ShaderParam* p) : m_Param(p)
		{
		}
	};

private:
	u32 m_RegisterVS;
	u32 m_RegisterPS;

	u32 m_RegisterPSCount;
	u32 m_RegisterVSCount;

	core::Type m_Type;
	u8 m_TypeSize;
	const char* m_Name;

	Shader* m_Owner;

public:
	ShaderParam(Shader *owner,
		core::Type type, u8 typeSize,
		const char* name,
		u32 registerVS, u32 registerPS,
		u32 registerVSCount, u32 registerPSCount) :
		m_RegisterVS(registerVS), m_RegisterPS(registerPS),
		m_RegisterVSCount(registerVSCount), m_RegisterPSCount(registerPSCount),
		m_Type(type), m_TypeSize(typeSize),
		m_Name(name),
		m_Owner(owner)
	{
	}

	ShaderParam() : 
		m_RegisterVS(0xFFFFFFFF), m_RegisterPS(0xFFFFFFFF),
		m_RegisterVSCount(0), m_RegisterPSCount(0),
		m_Type(core::Type::Unknown), m_TypeSize(0),
		m_Name(nullptr),
		m_Owner(nullptr)
	{
	}

	ShaderParam(const ShaderParam& other) : 
		m_RegisterVS(other.m_RegisterVS),
		m_RegisterPS(other.m_RegisterPS),
		m_RegisterVSCount(other.m_RegisterVSCount),
		m_RegisterPSCount(other.m_RegisterPSCount),
		m_Type(other.m_Type), m_TypeSize(other.m_TypeSize),
		m_Name(other.m_Name),
		m_Owner(other.m_Owner)
	{
	}

	bool IsValid() const
	{
		return (m_RegisterPS != 0xFFFFFFFF) || (m_RegisterVS != 0xFFFFFFFF);
	}

	core::Type GetType() const
	{
		return m_Type;
	}

	const char* GetName() const
	{
		return m_Name;
	}

	ShaderValueAccess operator*() const
	{
		return ShaderValueAccess(this);
	}

	ShaderParam& operator=(const ShaderParam& other)
	{
		m_Owner = other.m_Owner;
		m_Name = other.m_Name;
		m_RegisterPS = other.m_RegisterPS;
		m_RegisterVS = other.m_RegisterVS;

		m_RegisterPSCount = other.m_RegisterPSCount;
		m_RegisterVSCount = other.m_RegisterVSCount;

		m_Type = other.m_Type;
		m_TypeSize = other.m_TypeSize;

		return *this;
	}

	void GetShaderValue(void* out) const
	{
		m_Owner->GetShaderValue(
			m_RegisterVS, m_RegisterPS, 
			m_RegisterVSCount, m_RegisterPSCount,
			m_Type, m_TypeSize, out);
	}

	void SetShaderValue(const void* data) const
	{
		m_Owner->SetShaderValue(
			m_RegisterVS, m_RegisterPS,
			m_RegisterVSCount, m_RegisterPSCount,
			m_Type, m_TypeSize, data);
	}
};


inline ShaderParam::ShaderValueAccess& ShaderParam::ShaderValueAccess::operator= (const ShaderParam::ShaderValueAccess& value)
{
	if(m_Param->IsValid() && value.m_Param->IsValid() &&
		m_Param->m_Owner == value.m_Param->m_Owner &&
		m_Param->m_Type == value.m_Param->m_Type &&
		m_Param->m_Type != core::Type::Unknown) {
		static u8 data[64];

		value.m_Param->GetShaderValue(data);
		m_Param->SetShaderValue(data);
	}

	return *this;
}

inline ShaderParam::ShaderValueAccess& ShaderParam::ShaderValueAccess::operator= (bool value)
{
	m_Param->SetShaderValue(&value);
	return *this;
}

inline ShaderParam::ShaderValueAccess& ShaderParam::ShaderValueAccess::operator= (int value)
{
	m_Param->SetShaderValue(&value);
	return *this;
}

inline ShaderParam::ShaderValueAccess& ShaderParam::ShaderValueAccess::operator= (float value)
{
	m_Param->SetShaderValue(&value);
	return *this;
}

inline ShaderParam::ShaderValueAccess& ShaderParam::ShaderValueAccess::operator= (const math::vector2f& value)
{
	float a[2] = {value.x, value.y};
	m_Param->SetShaderValue(a);
	return *this;
}

inline ShaderParam::ShaderValueAccess& ShaderParam::ShaderValueAccess::operator= (const math::vector3f& value)
{
	float a[3] = {value.x, value.y, value.z};
	m_Param->SetShaderValue(a);
	return *this;
}

inline ShaderParam::ShaderValueAccess& ShaderParam::ShaderValueAccess::operator= (const video::Colorf& value)
{
	float a[4] = {value.r, value.g, value.b, value.a};
	m_Param->SetShaderValue(a);
	return *this;
}

inline ShaderParam::ShaderValueAccess& ShaderParam::ShaderValueAccess::operator= (const math::matrix4& value)
{
	m_Param->SetShaderValue(value.DataRowMajor());
	return *this;
}

inline ShaderParam::ShaderValueAccess::operator bool()
{
	bool out;
	m_Param->GetShaderValue(&out);

	return out;
}

inline ShaderParam::ShaderValueAccess::operator int()
{
	int out;
	m_Param->GetShaderValue(&out);

	return out;
}

inline ShaderParam::ShaderValueAccess::operator float()
{
	float out;
	m_Param->GetShaderValue(&out);

	return out;
}

inline ShaderParam::ShaderValueAccess::operator math::vector2f()
{
	float a[2];
	m_Param->GetShaderValue(a);

	return math::vector2f(a[0], a[1]);
}

inline ShaderParam::ShaderValueAccess::operator math::vector3f()
{
	float a[3];
	m_Param->GetShaderValue(a);

	return math::vector3f(a[0], a[1], a[2]);
}

inline ShaderParam::ShaderValueAccess::operator video::Colorf()
{
	video::Colorf out;
	float a[4];
	m_Param->GetShaderValue(a);

	return video::Colorf(a[0], a[1], a[2], a[3]);
}

inline ShaderParam::ShaderValueAccess::operator math::matrix4()
{
	math::matrix4 out;
	m_Param->GetShaderValue(out.DataRowMajor());

	return out;
}

inline float ShaderParam::ShaderValueAccess::Matrix(int row, int col)
{
	return ((math::matrix4)(*this))(row, col);
}

}

}

#endif
