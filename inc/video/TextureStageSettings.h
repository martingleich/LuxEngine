#ifndef INCLUDED_TEXTURE_STAGE_SETTINGS_H
#define INCLUDED_TEXTURE_STAGE_SETTINGS_H
#include "core/LuxBase.h"
#include "video/VideoEnums.h"

namespace lux
{
namespace video
{

class TextureStageSettings
{
public:
	u32 coordSource;

	ETextureArgument colorArg1;
	ETextureArgument colorArg2;
	ETextureOperator colorOperator;

	ETextureArgument alphaArg1;
	ETextureArgument alphaArg2;
	ETextureOperator alphaOperator;

	TextureStageSettings() :
		coordSource(0xFFFFFFFF),
		colorArg1(ETextureArgument::Diffuse),
		colorArg2(ETextureArgument::Texture),
		colorOperator(ETextureOperator::Modulate),
		alphaArg1(ETextureArgument::Diffuse),
		alphaArg2(ETextureArgument::Texture),
		alphaOperator(ETextureOperator::Modulate)
	{
	}

	TextureStageSettings(
		ETextureArgument _colorArg1,
		ETextureArgument _colorArg2,
		ETextureOperator _colorOperator,
		ETextureArgument _alphaArg1,
		ETextureArgument _alphaArg2,
		ETextureOperator _alphaOperator,
		u32 _coordSource = 0xFFFFFFFF) :
		coordSource(_coordSource),
		colorArg1(_colorArg1),
		colorArg2(_colorArg2),
		colorOperator(_colorOperator),
		alphaArg1(_alphaArg1),
		alphaArg2(_alphaArg2),
		alphaOperator(_alphaOperator)
	{
	}

	bool HasAlternateCoordSource() const
	{
		return coordSource != 0xFFFFFFFF;
	}

	bool operator==(const TextureStageSettings& other) const
	{
		return
			coordSource == other.coordSource &&
			colorArg1 == other.colorArg1 &&
			colorArg2 == other.colorArg2 &&
			colorOperator == other.colorOperator &&
			alphaArg1 == other.alphaArg1 &&
			alphaArg2 == other.alphaArg2 &&
			alphaOperator == other.alphaOperator;
	}
	bool operator!=(const TextureStageSettings& other) const
	{
		return !(*this == other);
	}
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_TEXTURE_STAGE_SETTINGS_H