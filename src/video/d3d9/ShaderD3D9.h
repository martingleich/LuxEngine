#ifndef INCLUDED_LUX_SHADER_D3D9_H
#define INCLUDED_LUX_SHADER_D3D9_H
#include "LuxConfig.h"
#ifdef LUX_COMPILE_WITH_D3D9
#include "video/Shader.h"
#include "core/lxMemory.h"
#include "core/ParamPackage.h"

#include "platform/StrippedD3D9.h"
#include "platform/StrippedD3D9X.h"
#include "platform/UnknownRefCounted.h"

namespace lux
{
namespace video
{
class VideoDriver;
class Renderer;
class DeviceStateD3D9;

class ShaderD3D9 : public Shader
{
public:
	ShaderD3D9(VideoDriver* driver, DeviceStateD3D9& deviceState);
	~ShaderD3D9();

	bool Init(
		core::StringView vsCode, core::StringView vsEntryPoint, core::StringView vsProfile,
		core::StringView psCode, core::StringView psEntryPoint, core::StringView psProfile,
		core::Array<core::String>* errorList);

	void Enable();
	void SetParam(int paramId, const void* data);
	int GetParamId(core::StringView name) const;
	void LoadSceneParams(const Pass& pass);
	void Render() {}
	void Disable();

	int GetSceneParamCount() const;
	core::AttributePtr GetSceneParam(int id) const;

	const core::ParamPackage& GetParamPackage() const;

	bool IsFixedFunction() const
	{
		return false;
	}

private:
	enum EParamType
	{
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
		ColorF,
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
		DefaultParam_COUNT
	};

	struct HelperEntry
	{
		u32 registerVS;
		u32 registerPS;

		u32 registerVSCount;
		u32 registerPSCount;

		EType type;
		u8 typeSize;
		core::StringView name;
		const void* defaultValue;

		EParamType paramType;
		u32 samplerStage;

		HelperEntry() :
			registerVS(0xFFFFFFFF),
			registerPS(0xFFFFFFFF),
			registerVSCount(0),
			registerPSCount(0),
			type(EType::Unknown),
			typeSize(0),
			defaultValue(nullptr),
			paramType(ParamType_ParamMaterial),
			samplerStage(0)
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
		int index;
		u32 samplerStage;
		core::AttributePtr sceneValue;
	};

private:
	bool GetStructureElemType(D3DXHANDLE structHandle, ID3DXConstantTable* table, u32& samplerStage, EType& outType, u32& outSize, u32& registerID, u32& regCount, core::StringView& name, const void*& defaultValue, bool& isValid);

	void LoadAllParams(bool isVertex, ID3DXConstantTable* table, core::Array<HelperEntry>& outParams, u32& outStringSize, core::Array<core::String>* errorList);

	UnknownRefCounted<IDirect3DPixelShader9> CreatePixelShader(core::StringView code, core::StringView entryPoint, core::StringView profile,
		core::Array<core::String>* errorList, UnknownRefCounted<ID3DXConstantTable>& outTable);
	UnknownRefCounted<IDirect3DVertexShader9> CreateVertexShader(core::StringView code, core::StringView entryPoint, core::StringView profile,
		core::Array<core::String>* errorList, UnknownRefCounted<ID3DXConstantTable>& outTable);

	void SetShaderValue(const Param& p, const void* data);

	void CastTypeToShader(EType type, const void* in, void* out);
	void CastShaderToType(EType type, const void* in, void* out);

	static bool IsTypeCompatible(EType a, EType b);
	static core::Type GetCoreType(EType type);

private:
	IDirect3DDevice9* m_D3DDevice;
	Renderer* m_Renderer;
	DeviceStateD3D9& m_DeviceState;

	UnknownRefCounted<IDirect3DVertexShader9> m_VertexShader;
	UnknownRefCounted<IDirect3DPixelShader9> m_PixelShader;

	core::Array<Param> m_Params;
	core::Array<Param> m_SceneValues;

	core::RawMemory m_Names;
	core::ParamPackage m_ParamPackage;
};

} // namespace video
} // namespace lux

#endif // LUX_COMPILE_WITH_D3D9

#endif