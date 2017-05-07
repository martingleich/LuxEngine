#ifndef INCLUDED_SHADER_D3D9_H
#define INCLUDED_SHADER_D3D9_H
#include "video/Shader.h"
#include "core/lxHashMap.h"
#include "video/SceneValues.h"
#include "core/lxMemory.h"

#ifdef LUX_COMPILE_WITH_D3D9

#include "StrippedD3D9.h"
#include "StrippedD3D9X.h"
#include "video/d3d9/UnknownRefCounted.h"

namespace lux
{
namespace video
{

class VideoDriver;
class ShaderD3D9 : public Shader
{
public:
	ShaderD3D9(VideoDriver* driver);

	void Init(
		const char* vsCode, const char* vsEntryPoint, size_t vsLength, const char* vsProfile,
		const char* psCode, const char* psEntryPoint, size_t psLength, const char* psProfile,
		core::array<string>* errorList);

	const ShaderParam& GetParam(const char* name);
	const ShaderParam& GetParam(u32 index);
	u32 GetParamCount() const;

	void Enable();
	void LoadParams(const core::PackagePuffer& puffer, const RenderData* renderData);
	void LoadSceneValues();
	void Disable();

	core::ParamPackage& GetParamPackage();

private:
	enum EParamType
	{
		ParamType_DefaultMaterial,
		ParamType_ParamMaterial,
		ParamType_Scene,
	};

	enum EDefaultParam
	{
		DefaultParam_Shininess = 0,
		DefaultParam_Diffuse = 1,
		DefaultParam_Emissive = 2,
		DefaultParam_Specular = 3,
	};

	struct ParamEntry
	{
		ShaderParam param;
		EParamType paramType;
		u32 index;

		ParamEntry()
		{
		}

		ParamEntry(EParamType t, const ShaderParam& p, u32 id = 0) :
			param(p),
			paramType(t),
			index(id)
		{
		}
	};

	struct HelperEntry
	{
		u32 registerVS;
		u32 registerPS;

		u32 registerVSCount;
		u32 registerPSCount;

		core::Type type;
		u8 typeSize;
		const char* name;
		u32 nameLength;
		const void* defaultValue;

		EParamType paramType;

		HelperEntry() :
			registerVS(0xFFFFFFFF),
			registerPS(0xFFFFFFFF),
			registerVSCount(0),
			registerPSCount(0),
			type(core::Type::Unknown),
			typeSize(0),
			name(nullptr),
			nameLength(0),
			defaultValue(nullptr),
			paramType(ParamType_ParamMaterial)
		{
		}
	};

private:
	bool GetStructureElemType(D3DXHANDLE structHandle, ID3DXConstantTable* table, core::Type& outType, u32& outSize, u32& registerID, u32& regCount, const char*& name, const void*& defaultValue, bool& isValid);

	void LoadAllParams(bool isVertex, ID3DXConstantTable* table, core::array<HelperEntry>& outParams, u32& outStringSize, core::array<string>* errorList);

	UnknownRefCounted<IDirect3DPixelShader9> CreatePixelShader(const char* code, const char* entryPoint, size_t length, const char* profile,
		core::array<string>* errorList, UnknownRefCounted<ID3DXConstantTable>& outTable);
	UnknownRefCounted<IDirect3DVertexShader9> CreateVertexShader(const char* code, const char* entryPoint, size_t length, const char* profile,
		core::array<string>* errorList, UnknownRefCounted<ID3DXConstantTable>& outTable);

	void GetShaderValue(
		u32 registerVS, u32 registerPS,
		u32 registerVSCount, u32 registerPSCount,
		core::Type type, u32 size, void* out);
	void SetShaderValue(
		u32 registerVS, u32 registerPS,
		u32 registerVSCount, u32 registerPSCount,
		core::Type type, u32 size, const void* data);

	void CastTypeToShader(core::Type type, const void* in, void* out);
	void CastShaderToType(core::Type type, const void* in, void* out);

	static int GetDefaultId(const char* name);
	static core::Type GetDefaultType(u32 id);

	static bool IsTypeCompatible(core::Type a, core::Type b);

private:
	IDirect3DDevice9* m_D3DDevice;
	scene::SceneValues* m_SceneValues;

	UnknownRefCounted<IDirect3DVertexShader9> m_VertexShader;
	UnknownRefCounted<IDirect3DPixelShader9> m_PixelShader;

	core::array<ParamEntry> m_Params;

	core::mem::RawMemory m_Names;
	core::ParamPackage m_ParamPackage;

	ShaderParam m_InvalidParam;
};

}

}

#endif // LUX_COMPILE_WITH_D3D9


#endif