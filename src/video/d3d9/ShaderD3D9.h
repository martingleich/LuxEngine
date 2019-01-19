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
private:
	enum class EParamType
	{
		Other,
		Param,
		Scene,
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

	struct RegisterLocation
	{
		u32 id;
		u32 count = 0;
	};

	struct Param
	{
		Param() {}

		RegisterLocation vsLocation;
		RegisterLocation psLocation;

		EType type = EType::Unknown;

		EParamType paramType = EParamType::Other;

		u32 samplerStage;

		core::StringView name;
		const void* defaultValue = nullptr;

		core::AttributePtr sceneValue;
	};

public:
	ShaderD3D9(VideoDriver* driver, DeviceStateD3D9& deviceState);
	~ShaderD3D9();

	bool Init(
		core::StringView vsCode, core::StringView vsEntryPoint, core::StringView vsProfile,
		core::StringView psCode, core::StringView psEntryPoint, core::StringView psProfile,
		core::Array<core::String>* errorList);

	void Enable() override;
	void SetParam(int paramId, const void* data) override;
	void LoadSceneParams(core::AttributeList sceneAttributes, const Pass& pass) override;
	void Render() override {}
	void Disable() override;

	const core::ParamPackage& GetParamPackage() const;

private:
	UnknownRefCounted<IDirect3DPixelShader9> CreatePixelShader(core::StringView code, core::StringView entryPoint, core::StringView profile,
		core::Array<core::String>* errorList, UnknownRefCounted<ID3DXConstantTable>& outTable);
	UnknownRefCounted<IDirect3DVertexShader9> CreateVertexShader(core::StringView code, core::StringView entryPoint, core::StringView profile,
		core::Array<core::String>* errorList, UnknownRefCounted<ID3DXConstantTable>& outTable);

	bool LoadAllParams(
		bool isVertex,
		ID3DXConstantTable* table,
		core::Array<Param>& outParams,
		u32& outStringSize,
		core::Array<core::String>* errorList);

	bool GetParamInfo(
		D3DXHANDLE structHandle,
		ID3DXConstantTable* table,
		u32& samplerStage,
		EType& outType,
		RegisterLocation& location,
		core::StringView& name,
		const void*& defaultValue,
		EParamType& paramType);

	void SetShaderValue(const Param& p, const void* data);

	void CastTypeToShader(EType type, const void* in, void* out);
	void CastShaderToType(EType type, const void* in, void* out);

	static core::Type GetCoreType(EType type);
	static EType GetTypeFromD3DXDesc(const D3DXCONSTANT_DESC& desc);

private:
	IDirect3DDevice9* m_D3DDevice;
	DeviceStateD3D9& m_DeviceState;

	UnknownRefCounted<IDirect3DVertexShader9> m_VertexShader;
	UnknownRefCounted<IDirect3DPixelShader9> m_PixelShader;

	core::Array<Param> m_Params;
	core::Array<Param> m_SceneValues;

	core::RawMemory m_Names;
	core::ParamPackage m_ParamPackage;

	mutable core::AttributeList m_CurAttributes;
};

} // namespace video
} // namespace lux

#endif // LUX_COMPILE_WITH_D3D9

#endif