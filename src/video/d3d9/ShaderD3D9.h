#ifndef INCLUDED_LUX_SHADER_D3D9_H
#define INCLUDED_LUX_SHADER_D3D9_H
#include "LuxConfig.h"

#ifdef LUX_COMPILE_WITH_D3D9
#include "video/Shader.h"
#include "core/ParamPackage.h"

#include "platform/StrippedD3D9.h"
#include "platform/UnknownRefCounted.h"

namespace lux
{
namespace video
{

class DeviceStateD3D9;

struct ShaderCompileRequest_HLSL_D3DX
{
	core::StringView vsCode;
	core::StringView vsProfile;
	core::StringView psCode;
	core::StringView psProfile;
};

StrongRef<Shader> Compile_HLSL_D3DX(
	DeviceStateD3D9& deviceState,
	IDirect3DDevice9* device,
	const ShaderCompileRequest_HLSL_D3DX& req,
	core::Array<ShaderCompileMessage>& messages);

class ShaderD3D9 : public Shader
{
public:
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

	struct ParamLocation
	{
		/*
		Register index or samplerStage
		*/
		u32 id;
		u32 count = 0;
	};

	struct BasicParam
	{
		ParamLocation vsLocation;
		ParamLocation psLocation;

		EType type = EType::Unknown;
	};
	struct Param : BasicParam
	{};

	struct SceneParam : BasicParam
	{
		core::String name;
	};

public:
	ShaderD3D9(
		DeviceStateD3D9& deviceState,
		UnknownRefCounted<IDirect3DVertexShader9> vsShader,
		UnknownRefCounted<IDirect3DPixelShader9> psShader,
		const core::ParamPackage& paramPackage,
		const core::Array<Param>& params,
		const core::Array<SceneParam>& sceneParams);
	~ShaderD3D9();

	void Enable() override;
	void SetParam(int paramId, const void* data) override;
	void LoadSceneParams(core::AttributeList sceneAttributes, const Pass& pass) override;
	// TODO: Cache params and apply all at once in Render()
	// This reduces the number of driver calls. and makes caching easier.
	// Handle settings with DeviceState.
	// Maybe completly remove Enable/Disable/Render and handle all in DeviceState.
	void Render() override {}
	void Disable() override;

	const core::ParamPackage& GetParamPackage() const;

private:
	void SetShaderValue(const BasicParam& p, const void* data);

private:
	DeviceStateD3D9& m_DeviceState;

	UnknownRefCounted<IDirect3DVertexShader9> m_VertexShader;
	UnknownRefCounted<IDirect3DPixelShader9> m_PixelShader;

	core::Array<Param> m_Params;
	core::Array<SceneParam> m_SceneValues;
	core::ParamPackage m_ParamPackage;

	mutable core::AttributeList m_CurAttributes;
	mutable core::Array<core::AttributePtr> m_SceneValueAttributeCache;
};

} // namespace video
} // namespace lux

#endif // LUX_COMPILE_WITH_D3D9

#endif