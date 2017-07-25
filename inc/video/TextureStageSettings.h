#ifndef INCLUDED_TEXTURE_STAGE_SETTINGS_H
#define INCLUDED_TEXTURE_STAGE_SETTINGS_H

namespace lux
{
namespace video
{

enum class ETextureOperator
{
	Disable,
	SelectArg1,
	SelectArg2,
	Modulate,
	Add,
	AddSigned,
	AddSmoth,
	Subtract,
	Blend,
	Dot,
};

enum class ETextureArgument
{
	Current,
	Texture,
	Diffuse,
	AlphaRep,
};

class TextureStageSettings
{
public:
	ETextureArgument colorArg1;
	ETextureArgument colorArg2;
	ETextureOperator colorOperator;

	ETextureArgument alphaArg1;
	ETextureArgument alphaArg2;
	ETextureOperator alphaOperator;

	u32 coordSource;

	TextureStageSettings() :
		colorArg1(ETextureArgument::Diffuse),
		colorArg2(ETextureArgument::Texture),
		colorOperator(ETextureOperator::Modulate),
		alphaArg1(ETextureArgument::Diffuse),
		alphaArg2(ETextureArgument::Texture),
		alphaOperator(ETextureOperator::Modulate),
		coordSource(0xFFFFFFFF)
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
		colorArg1(_colorArg1),
		colorArg2(_colorArg2),
		colorOperator(_colorOperator),
		alphaArg1(_alphaArg1),
		alphaArg2(_alphaArg2),
		alphaOperator(_alphaOperator),
		coordSource(_coordSource)
	{
	}

	bool HasAlternateCoordSource() const
	{
		return coordSource != 0xFFFFFFFF;
	}
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_TEXTURE_STAGE_SETTINGS_H