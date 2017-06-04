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
		colorArg2(ETextureArgument::Diffuse),
		colorOperator(ETextureOperator::SelectArg1),
		alphaArg1(ETextureArgument::Diffuse),
		alphaArg2(ETextureArgument::Diffuse),
		alphaOperator(ETextureOperator::SelectArg1),
		coordSource(0xFFFFFFFF)
	{}

	bool HasAlternateCoordSource() const
	{
		return coordSource != 0xFFFFFFFF;
	}
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_TEXTURE_STAGE_SETTINGS_H