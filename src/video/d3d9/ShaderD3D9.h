#ifndef INCLUDED_SHADER_D3D9_H
#define INCLUDED_SHADER_D3D9_H
#ifdef LUX_COMPILE_WITH_D3D9
#include "video/Shader.h"
#include "core/lxMemory.h"
#include "core/ParamPackage.h"

#include "StrippedD3D9.h"
#include "StrippedD3D9X.h"
#include "video/d3d9/UnknownRefCounted.h"

namespace lux
{
namespace video
{
class RenderSettings;
class VideoDriver;
class Renderer;

class ShaderD3D9 : public Shader
{
public:
	ShaderD3D9(VideoDriver* driver);

	void Init(
		const char* vsCode, const char* vsEntryPoint, size_t vsLength, const char* vsProfile,
		const char* psCode, const char* psEntryPoint, size_t psLength, const char* psProfile,
		core::array<string>* errorList);

	void Enable();
	void LoadSettings(const RenderSettings& settings);
	void Disable();

	const core::ParamPackage& GetParamPackage() const;

	size_t GetSceneParamCount() const;
	u32 GetSceneParam(size_t id) const;

private:
	enum EParamType
	{
		ParamType_DefaultMaterial,
		ParamType_ParamMaterial,
		ParamType_Scene,
	};

	enum class EType
	{
		Unknown,

		Integer,
		Float,
		Boolean,
		U32,

		Texture,
		Color,
		Colorf,
		Vector2,
		Vector3,
		Vector2Int,
		Vector3Int,
		Matrix,

		Matrix_ColMajor,
		Structure,
	};

	enum EDefaultParam
	{
		DefaultParam_Shininess = 0,
		DefaultParam_Diffuse = 1,
		DefaultParam_Emissive = 2,
		DefaultParam_Specular = 3,
	};

	struct HelperEntry
	{
		u32 registerVS;
		u32 registerPS;

		u32 registerVSCount;
		u32 registerPSCount;

		EType type;
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
			type(EType::Unknown),
			typeSize(0),
			name(nullptr),
			nameLength(0),
			defaultValue(nullptr),
			paramType(ParamType_ParamMaterial)
		{
		}
	};

	struct Param
	{
		Param() {}

		u32 registerVS;
		u32 registerPS;

		u32 registerVSCount;
		u32 registerPSCount;

		EType type;

		EParamType paramType;
		u32 index;
	};

private:
	bool GetStructureElemType(D3DXHANDLE structHandle, ID3DXConstantTable* table, EType& outType, u32& outSize, u32& registerID, u32& regCount, const char*& name, const void*& defaultValue, bool& isValid);

	void LoadAllParams(bool isVertex, ID3DXConstantTable* table, core::array<HelperEntry>& outParams, u32& outStringSize, core::array<string>* errorList);

	UnknownRefCounted<IDirect3DPixelShader9> CreatePixelShader(const char* code, const char* entryPoint, size_t length, const char* profile,
		core::array<string>* errorList, UnknownRefCounted<ID3DXConstantTable>& outTable);
	UnknownRefCounted<IDirect3DVertexShader9> CreateVertexShader(const char* code, const char* entryPoint, size_t length, const char* profile,
		core::array<string>* errorList, UnknownRefCounted<ID3DXConstantTable>& outTable);

	void SetShaderValue(const Param& p, const void* data);

	void CastTypeToShader(EType type, const void* in, void* out);
	void CastShaderToType(EType type, const void* in, void* out);

	static int GetDefaultId(const char* name);
	static EType GetDefaultType(u32 id);

	static bool IsTypeCompatible(EType a, EType b);

	static core::Type GetCoreType(EType type);

private:
	IDirect3DDevice9* m_D3DDevice;
	video::Renderer* m_Renderer;

	UnknownRefCounted<IDirect3DVertexShader9> m_VertexShader;
	UnknownRefCounted<IDirect3DPixelShader9> m_PixelShader;

	core::array<Param> m_Params;
	core::array<Param> m_SceneValues;

	core::mem::RawMemory m_Names;
	core::ParamPackage m_ParamPackage;
};

} // namespace video
} // namespace lux

#endif // LUX_COMPILE_WITH_D3D9

#endif