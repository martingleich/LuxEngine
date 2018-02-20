#ifndef INCLUDED_PASS_H
#define INCLUDED_PASS_H
#include "core/LuxBase.h"
#include "video/TextureLayer.h"
#include "video/TextureStageSettings.h"
#include "core/lxArray.h"
#include "core/lxString.h"
#include "video/Shader.h"
#include "core/StringConverter.h"

namespace lux
{
namespace video
{
enum class EMaterialRequirement
{
	None = 0,
	Transparent = 1,
	DeferredEffect = 2,
};

enum class EPipelineSetting
{
	Stencil,
	PolygonOffset,
	AlphaBlending,
	ZBufferFunc,
	ColorMask,
	DrawMode,
	Lighting,
	Fog,
	ZWrite,
	NormalizeNormals,
	GourandShading,
	Culling,
	UseVertexColor,
};

enum class EOptionId
{
	Layer0,
	Layer1,
	Layer2,
	Layer3,
	Layer4,
	Layer5,
	Layer6,
	Layer7,
};

#define EOptionId_Layer(n) EOptionId((int)EOptionId::Layer0+n)

class Pass
{
public:
	Pass() :
		fogEnabled(true),
		zWriteEnabled(true),
		normalizeNormals(false),
		gouraudShading(true),
		useVertexColor(false)
	{
	}

	// Material
	float ambient = 1.0f;
	Colorf diffuse = video::Colorf(1, 1, 1, 1);
	Colorf emissive = video::Colorf(0, 0, 0, 0);
	Colorf specular = video::Colorf(1, 1, 1, 1);
	float shininess = 0;

	// Textures
	core::Array<TextureLayer> layers;
	core::Array<TextureStageSettings> layerSettings;

	// Shader
	StrongRef<Shader> shader;

	// Pipelinesettings
	StencilMode stencil;

	float polygonOffset = 0.0f;

	AlphaBlendMode alpha;

	EComparisonFunc zBufferFunc = EComparisonFunc::LessEqual;
	u32 colorMask = 0xFFFFFFFF;
	EDrawMode drawMode = EDrawMode::Fill;
	ELighting lighting = ELighting::Enabled;
	EFaceSide culling = EFaceSide::Back;

	bool fogEnabled : 1;
	bool zWriteEnabled : 1;
	bool normalizeNormals : 1;
	bool gouraudShading : 1;
	bool useVertexColor : 1;

	// Functions
	TextureLayer& AddTexture()
	{
		layers.EmplaceBack();
		return layers.Back();
	}
	TextureStageSettings& AddStage()
	{
		layerSettings.EmplaceBack();
		return layerSettings.Back();
	}

	u32 GetOptionCount() const
	{
		return layers.Size();
	}

	const core::String& GetOptionName(u32 id) const
	{
		static core::Array<core::String> layerNames;
		if(id >= layers.Size())
			throw core::OutOfRangeException();
		if(id >= layerNames.Size()) {
			for(size_t i = layerNames.Size(); i < id; ++i)
				layerNames.PushBack(
					"layer" + core::StringConverter::ToString(i));
		}

		return layerNames[id];
	}

	core::Type GetOptionType(u32 id) const
	{
		if(id >= layers.Size())
			throw core::OutOfRangeException();
		return core::Types::Texture();
	}

	void SetOption(u32 id, const void* data)
	{
		if(id >= layers.Size())
			throw core::OutOfRangeException();
		GetOptionType(id).CopyConstruct(&layers[id], data);
	}
};

class ShaderParamSetCallback
{
public:
	virtual ~ShaderParamSetCallback() {}

	virtual void SendShaderSettings(const Pass& pass, void* userParam) const
	{
		LUX_UNUSED(pass);
		LUX_UNUSED(userParam);
	}
	virtual void SendShaderSettings(u32 passId, const Pass& pass, void* userParam) const
	{
		LUX_UNUSED(passId);
		SendShaderSettings(pass, userParam);
	}
};

class PipelineOverwrite
{
public:
	// Bitset for enabled flags.
	u32 enabledOverwrites;

	// Pipelinesettings.
	StencilMode stencil;

	float polygonOffset;

	AlphaBlendMode alpha;

	EComparisonFunc zBufferFunc;
	u32 colorMask;
	EDrawMode drawMode;
	ELighting lighting;
	EFaceSide culling;

	bool fogEnabled : 1;
	bool zWriteEnabled : 1;
	bool normalizeNormals : 1;
	bool gouraudShading : 1;
	bool useVertexColor : 1;

	PipelineOverwrite() :
		enabledOverwrites(0)
	{
	}

	PipelineOverwrite& Enable(EPipelineSetting setting)
	{
		enabledOverwrites |= 1 << ((u32)setting);
		return *this;
	}
	PipelineOverwrite& Disable(EPipelineSetting setting)
	{
		enabledOverwrites &= ~(1 << ((u32)setting));
		return *this;
	}
	bool IsEnabled(EPipelineSetting setting) const
	{
		return (enabledOverwrites & (1 << (u32)setting)) != 0;
	}

	void Append(const PipelineOverwrite& next)
	{
		enabledOverwrites |= next.enabledOverwrites;
		if(next.IsEnabled(EPipelineSetting::Stencil))
			stencil = next.stencil;
		if(next.IsEnabled(EPipelineSetting::AlphaBlending))
			alpha = next.alpha;
		if(next.IsEnabled(EPipelineSetting::PolygonOffset))
			polygonOffset += next.polygonOffset;
		if(next.IsEnabled(EPipelineSetting::Lighting))
			lighting = next.lighting;
		if(next.IsEnabled(EPipelineSetting::Fog))
			fogEnabled = next.fogEnabled;
		if(next.IsEnabled(EPipelineSetting::DrawMode))
			drawMode = next.drawMode;
		if(next.IsEnabled(EPipelineSetting::ColorMask))
			colorMask &= next.colorMask;
		if(next.IsEnabled(EPipelineSetting::ZBufferFunc))
			zBufferFunc = next.zBufferFunc;
		if(next.IsEnabled(EPipelineSetting::ZWrite))
			zWriteEnabled = next.zWriteEnabled;
		if(next.IsEnabled(EPipelineSetting::NormalizeNormals))
			normalizeNormals = next.normalizeNormals;
		if(next.IsEnabled(EPipelineSetting::Culling))
			culling = next.culling;
	}

	void Apply(Pass& pass)
	{
		if(IsEnabled(EPipelineSetting::Stencil))
			pass.stencil = stencil;
		if(IsEnabled(EPipelineSetting::AlphaBlending))
			pass.alpha = alpha;
		if(IsEnabled(EPipelineSetting::PolygonOffset))
			pass.polygonOffset += polygonOffset;
		if(IsEnabled(EPipelineSetting::Lighting))
			pass.lighting = lighting;
		if(IsEnabled(EPipelineSetting::Fog))
			pass.fogEnabled = fogEnabled;
		if(IsEnabled(EPipelineSetting::DrawMode))
			pass.drawMode = drawMode;
		if(IsEnabled(EPipelineSetting::ColorMask))
			pass.colorMask &= colorMask;
		if(IsEnabled(EPipelineSetting::ZBufferFunc))
			pass.zBufferFunc = zBufferFunc;
		if(IsEnabled(EPipelineSetting::ZWrite))
			pass.zWriteEnabled = zWriteEnabled;
		if(IsEnabled(EPipelineSetting::NormalizeNormals))
			pass.normalizeNormals = normalizeNormals;
		if(IsEnabled(EPipelineSetting::Culling))
			pass.culling = culling;
	}

	/*
	bool operator==(const PipelineOverwrite& other) const
	{
		return
			polygonOffset == other.polygonOffset &&
			lighting == other.lighting &&
			disableFog == other.disableFog &&
			drawMode == other.drawMode &&
			disableZWrite == other.disableZWrite &&
			normalizeNormals == other.normalizeNormals &&
			disableBackfaceCulling == other.disableBackfaceCulling &&
			enableFrontfaceCulling == other.enableFrontfaceCulling;
	}

	bool operator!=(const PipelineOverwrite& other) const
	{
		return !(*this == other);
	}
	*/
};

} // namespace video

DECLARE_FLAG_CLASS(lux::video::EColorPlane);
DECLARE_FLAG_CLASS(lux::video::EMaterialRequirement);

} // namespace lux

#endif // #ifndef INCLUDED_PASS_H