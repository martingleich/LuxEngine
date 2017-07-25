#ifndef INCLUDED_PASS_H
#define INCLUDED_PASS_H
#include "core/LuxBase.h"
#include "video/TextureLayer.h"
#include "video/TextureStageSettings.h"
#include "video/AlphaSettings.h"
#include "core/lxArray.h"
#include "core/lxString.h"
#include "video/Shader.h"
#include "core/StringConverter.h"

namespace lux
{
namespace video
{

enum class EZComparisonFunc
{
	Never,
	Less,
	Equal,
	LessEqual,
	Greater,
	NotEqual,
	GreaterEqual,
	Always,
};

enum class EColorPlane // enum class flag at end of file
{
	None = 0,
	Alpha = 1,
	Red = 2,
	Green = 4,
	Blue = 8,
	RGB = Red | Green | Blue,
	All = RGB | Alpha,
};

enum class EDrawMode
{
	Fill,
	Wire,
	Point,
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
		isTransparent(false),
		alphaSrcBlend(EBlendFactor::One),
		alphaDstBlend(EBlendFactor::Zero),
		alphaOperator(EBlendOperator::None),
		zBufferFunc(EZComparisonFunc::LessEqual),
		colorPlane(EColorPlane::All),
		drawMode(EDrawMode::Fill),
		polygonOffset(0.0f),
		fogEnabled(true),
		lighting(true),
		zWriteEnabled(true),
		normalizeNormals(true),
		gouraudShading(true),
		backfaceCulling(true),
		frontfaceCulling(false),
		useVertexColor(false)
	{
	}

	bool isTransparent;

	core::Array<TextureLayer> layers;
	core::Array<TextureStageSettings> layerSettings;

	EBlendFactor alphaSrcBlend;
	EBlendFactor alphaDstBlend;
	EBlendOperator alphaOperator;

	EZComparisonFunc zBufferFunc;
	EColorPlane colorPlane;
	EDrawMode drawMode;
	float polygonOffset;

	bool fogEnabled : 1;
	bool lighting : 1;
	bool zWriteEnabled : 1;
	bool normalizeNormals : 1;
	bool gouraudShading : 1;
	bool backfaceCulling : 1;
	bool frontfaceCulling : 1;

	bool useVertexColor : 1;

	StrongRef<Shader> shader;

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
	bool disableLighting;
	bool disableFog;

	EDrawMode drawMode;

	bool disableZWrite;
	bool disableZCmp;
	bool normalizeNormals;

	bool disableBackfaceCulling;
	bool enableFrontfaceCulling;

	PipelineOverwrite() :
		polygonOffset(0.0f),
		disableLighting(false),
		disableFog(false),
		drawMode(EDrawMode::Fill),
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
		if(next.disableLighting)
			disableLighting = true;
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
		if(disableLighting)
			pass.lighting = false;
		if(disableFog)
			pass.fogEnabled = false;
		if(drawMode == EDrawMode::Wire)
			pass.drawMode = EDrawMode::Wire;
		if(drawMode == EDrawMode::Point)
			pass.drawMode = EDrawMode::Point;

		if(disableZCmp)
			pass.zBufferFunc = EZComparisonFunc::Always;
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
			disableLighting == other.disableLighting &&
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