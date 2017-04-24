#ifndef INCLUDED_SHADER_D3D9_H
#define INCLUDED_SHADER_D3D9_H
#include "video/Shader.h"
#include "core/lxHashMap.h"
#include "video/SceneValues.h"

#ifdef LUX_COMPILE_WITH_D3D9

#include "StrippedD3D9.h"
#include "StrippedD3D9X.h"

namespace lux
{
namespace video
{

class VideoDriver;
class ShaderImpl : public Shader
{
	friend class ShaderParam;

private:
	struct SParamEntry
	{
		ShaderParam Param;
		u32 index;    // The sceneindex

		SParamEntry(u32 _Index, const ShaderParam& p) : Param(p), index(_Index)
		{
		}

		SParamEntry()
		{
		}

		bool IsSceneValue() const
		{
			return (index != -1);
		}
	};

	struct SHelperEntry
	{
		u32            Register_VS;    // Das Register im Vertexshader
		u32            Register_PS;    // Das Register im Pixelshader
		core::Type    type;            // Der Datentyp
		u8            TypeSize;        // Die größe des Datentyps
		const char* pName;
		u32 NameLength;
		const void* default;

		SHelperEntry() : Register_VS(0xFFFFFFFF), Register_PS(0xFFFFFFFF), type(core::Type::Unknown), TypeSize(0), pName(nullptr), NameLength(0), default(nullptr)
		{
		}
	};

private:
	IDirect3DDevice9* m_D3DDevice;
	scene::SceneValues* m_SceneValues;
	IDirect3DVertexShader9* m_pVertexShader;
	IDirect3DPixelShader9* m_pPixelShader;

	ID3DXConstantTable* m_pVertexShaderConstants;
	ID3DXConstantTable* m_pPixelShaderConstants;

	ShaderParam m_paramInvalid;        // Fehlercode

	size_t m_MaterialParamCount;
	core::array<SParamEntry> m_Params;
	char* m_pNames;
	core::ParamPackage m_ParamPackage;

	/*
	Verlinken von Shader und Engine bzw. material
	material:
		Das ParamPackage speichert den zugehörigen ShaderParam-index zum verwenden ab
	Engine:
		Verknüfen mit ShaderManager
		Der Shader speichert für jeden ParamEntry den zugehörigen SceneParam ab(IDEA: Test existence of right value before usage)

	// Diese Parameter werden automatisch verknüpft
	float ambient;
	float shininess;
	Color diffuse;
	Color emissive;
	Color specular;
	*/

private:
	void GetStructureElemType(D3DXHANDLE StructHandle, u32 index, ID3DXConstantTable* Table, core::Type& outType, u32& outSize, u32& registerID, const char*& Nameconst, const void*& default);
	u32 LoadParams(ID3DXConstantTable* from, bool IsParam, core::array<SHelperEntry>& target, u32& StringSize, u32 ParamCount, u32 SceneCount);
	bool CreatePixelShader(const char* Code, const char* EntryPoint, size_t length, const char* Profile);
	bool CreateVertexShader(const char* Code, const char* EntryPoint, size_t length, const char* Profile);
	void GetShaderValue(u32 RegisterVS, u32 RegisterPS, core::Type type, u32 Size, void* out);
	void SetShaderValue(u32 RegisterVS, u32 RegisterPS, core::Type type, u32 Size, const void* data);
	void CastTypeToShader(core::Type type, const void* in, void* out);
	void CastShaderToType(core::Type type, const void* in, void* out);

public:
	ShaderImpl(VideoDriver* Driver);
	~ShaderImpl();

	bool Init(const char* VSCode, const char* VSEntryPoint, size_t VSLength, const char* VSProfile,
		const char* PSCode, const char* PSEntryPoint, size_t PSLength, const char* PSProfile);

	const ShaderParam& GetParam(const char* pcName);
	const ShaderParam& GetParam(u32 index);
	u32 GetParamCount() const;

	void Enable();
	void LoadParams(const core::PackagePuffer& Puffer);
	void Disable();

	const core::ParamPackage& GetParamPackage()
	{
		return m_ParamPackage;
	}
};

}

}

#endif // LUX_COMPILE_WITH_D3D9


#endif