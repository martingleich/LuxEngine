#ifndef INCLUDED_LUX_PASS_H
#define INCLUDED_LUX_PASS_H
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
	GourandShading,
	Culling,
};

class Pass
{
public:
	Pass() :
		fogEnabled(true),
		zWriteEnabled(true),
		gouraudShading(true)
	{
	}

	// Shader
	StrongRef<Shader> shader;

	// Pipelinesettings
	StencilMode stencil;

	float polygonOffset = 0.0f;

	AlphaBlendMode alpha;

	EComparisonFunc zBufferFunc = EComparisonFunc::LessEqual;
	u32 colorMask = 0xFFFFFFFF;
	EDrawMode drawMode = EDrawMode::Fill;
	ELightingFlag lighting = ELightingFlag::Enabled;
	EFaceSide culling = EFaceSide::Back;

	bool fogEnabled : 1;
	bool zWriteEnabled : 1;
	bool gouraudShading : 1;
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
};

class PipelineOverwrite
{
public:
	PipelineOverwrite() :
		enabledOverwrites(0)
	{
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
		if(IsEnabled(EPipelineSetting::Culling))
			pass.culling = culling;
	}

	void OverwriteStencil(StencilMode v) {
		stencil = v;
		Enable(EPipelineSetting::Stencil);
	}
	void OverwritePolygonOffset(float v) {
		polygonOffset = v;
		Enable(EPipelineSetting::PolygonOffset);
	}
	void OverwriteAlpha(AlphaBlendMode v) {
		alpha = v;
		Enable(EPipelineSetting::AlphaBlending);
	}
	void OverwriteZBufferFunc(EComparisonFunc v) {
		zBufferFunc = v;
		Enable(EPipelineSetting::ZBufferFunc);
	}
	void OverwriteColorMask(u32 v) {
		colorMask = v;
		Enable(EPipelineSetting::ColorMask);
	}
	void OverwriteDrawMode(EDrawMode v) {
		drawMode = v;
		Enable(EPipelineSetting::DrawMode);
	}
	void OverwriteLighting(ELightingFlag v) {
		lighting = v;
		Enable(EPipelineSetting::Lighting);
	}
	void OverwriteCulling(EFaceSide v) {
		culling = v;
		Enable(EPipelineSetting::Culling);
	}
	void OverwriteFog(bool v) {
		fogEnabled = v;
		Enable(EPipelineSetting::Fog);
	}
	void OverwriteZWrite(bool v) {
		zWriteEnabled = v;
		Enable(EPipelineSetting::ZWrite);
	}
	void OverwriteGourandShading(bool v) {
		gouraudShading = v;
		Enable(EPipelineSetting::GourandShading);
	}
private:
	PipelineOverwrite& Enable(EPipelineSetting setting)
	{
		enabledOverwrites |= 1 << ((u32)setting);
		return *this;
	}

private:
	// Bitset for enabled flags.
	u32 enabledOverwrites;

	// Pipelinesettings.
	StencilMode stencil;

	float polygonOffset;

	AlphaBlendMode alpha;

	EComparisonFunc zBufferFunc;
	u32 colorMask;
	EDrawMode drawMode;
	ELightingFlag lighting;
	EFaceSide culling;

	bool fogEnabled : 1;
	bool zWriteEnabled : 1;
	bool gouraudShading : 1;
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_LUX_PASS_H