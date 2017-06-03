#ifndef INCLUDED_PIPELINESETTINGS_H
#define INCLUDED_PIPELINESETTINGS_H
#include "core/LuxBase.h"

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

class PipelineSettings
{
public:
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

	PipelineSettings() :
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
		frontfaceCulling(false)
	{
	}

	bool operator!=(const PipelineSettings& other) const
	{
		return
			drawMode != other.drawMode ||
			gouraudShading != other.gouraudShading ||
			lighting != other.lighting ||
			zWriteEnabled != other.zWriteEnabled ||
			backfaceCulling != other.backfaceCulling ||
			frontfaceCulling != other.frontfaceCulling ||
			fogEnabled != other.fogEnabled ||
			zBufferFunc != other.zBufferFunc ||
			normalizeNormals != other.normalizeNormals ||
			colorPlane != other.colorPlane ||
			polygonOffset != other.polygonOffset;
	}

	bool operator==(const PipelineSettings& other) const
	{
		return !(*this != other);
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

	void Apply(PipelineSettings& settings) const
	{
		settings.polygonOffset += polygonOffset;
		if(disableLighting)
			settings.lighting = false;
		if(disableFog)
			settings.fogEnabled = false;
		if(drawMode == EDrawMode::Wire)
			settings.drawMode = EDrawMode::Wire;
		if(drawMode == EDrawMode::Point)
			settings.drawMode = EDrawMode::Point;

		if(disableZCmp)
			settings.zBufferFunc = video::EZComparisonFunc::Always;
		if(disableZWrite)
			settings.zWriteEnabled = false;
		if(normalizeNormals)
			settings.normalizeNormals = true;
		if(disableBackfaceCulling)
			settings.backfaceCulling = false;
		if(enableFrontfaceCulling)
			settings.frontfaceCulling = true;
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

#endif // INCLUDED_PIPELINESETTINGS_H
