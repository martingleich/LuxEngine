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
		isTransparent(false),
		fogEnabled(true),
		zWriteEnabled(true),
		normalizeNormals(true),
		gouraudShading(true),
		backfaceCulling(true),
		frontfaceCulling(false),
		useVertexColor(false)
	{
	}

	core::Array<TextureLayer> layers;
	core::Array<TextureStageSettings> layerSettings;

	StrongRef<Shader> shader;

	float polygonOffset = 0.0f;

	EBlendFactor alphaSrcBlend = EBlendFactor::One;
	EBlendFactor alphaDstBlend = EBlendFactor::Zero;
	EBlendOperator alphaOperator =  EBlendOperator::None;

	EComparisonFunc zBufferFunc = EComparisonFunc::LessEqual;
	EColorPlane colorPlane = EColorPlane::All;
	EDrawMode drawMode = EDrawMode::Fill;
	ELighting lighting = ELighting::Enabled;

	bool isTransparent : 1;
	bool fogEnabled : 1;
	bool zWriteEnabled : 1;
	bool normalizeNormals : 1;
	bool gouraudShading : 1;
	bool backfaceCulling : 1;
	bool frontfaceCulling : 1;

	bool useVertexColor : 1;

	u32 AddTexture(u32 count = 1)
	{
		u32 out = layers.Size();
		while(count--)
			layers.PushBack(TextureLayer());
		return out;
	}

	u32 GetOptionCount() const
	{
		return layers.Size();
	}

	const String& GetOptionName(u32 id) const
	{
		static core::Array<String> layerNames;
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

class PipelineOverwrite
{
public:
	float polygonOffset;
	EDrawMode drawMode;
	EColorPlane colorPlane;

	ELighting lighting;
	bool disableFog:1;

	bool disableZWrite:1;
	bool disableZCmp:1;
	bool normalizeNormals:1;

	bool disableBackfaceCulling:1;
	bool enableFrontfaceCulling:1;

	PipelineOverwrite() :
		polygonOffset(0.0f),
		drawMode(EDrawMode::Fill),
		colorPlane(EColorPlane::All),
		disableFog(false),
		disableZWrite(false),
		disableZCmp(false),
		normalizeNormals(false),
		disableBackfaceCulling(false),
		enableFrontfaceCulling(false)
	{
	}

	void Append(const PipelineOverwrite& next)
	{
		polygonOffset += next.polygonOffset;
		if(next.lighting != ELighting::Enabled)
			lighting = next.lighting;
		if(next.disableFog)
			disableFog = true;
		if(next.drawMode == EDrawMode::Wire)
			drawMode = EDrawMode::Wire;
		if(next.drawMode == EDrawMode::Point)
			drawMode = EDrawMode::Point;

		if(next.disableZCmp)
			disableZCmp = true;
		if(next.disableZWrite)
			disableZWrite = true;
		if(next.normalizeNormals)
			normalizeNormals = true;
		if(next.disableBackfaceCulling)
			disableBackfaceCulling = true;
		if(next.enableFrontfaceCulling)
			enableFrontfaceCulling = true;
	}

	void Apply(Pass& pass)
	{
		pass.polygonOffset += polygonOffset;
		if(lighting != ELighting::Enabled)
			pass.lighting = lighting;
		if(disableFog)
			pass.fogEnabled = false;
		if(drawMode == EDrawMode::Wire)
			pass.drawMode = EDrawMode::Wire;
		if(drawMode == EDrawMode::Point)
			pass.drawMode = EDrawMode::Point;

		if(disableZCmp)
			pass.zBufferFunc = EComparisonFunc::Always;
		if(disableZWrite)
			pass.zWriteEnabled = false;
		if(normalizeNormals)
			pass.normalizeNormals = true;
		if(disableBackfaceCulling)
			pass.backfaceCulling = false;
		if(enableFrontfaceCulling)
			pass.frontfaceCulling = false;
	}

	bool operator==(const PipelineOverwrite& other) const
	{
		return polygonOffset == other.polygonOffset &&
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
};

} // namespace video

DECLARE_FLAG_CLASS(lux::video::EColorPlane);

} // namespace lux

#endif // #ifndef INCLUDED_PASS_H