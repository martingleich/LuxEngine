#ifndef INCLUDED_EDRIVER_CAPS_H
#define INCLUDED_EDRIVER_CAPS_H

namespace lux
{
namespace video
{

enum class EDriverCaps
{
	//! The maximum number of primitives per DrawCall
	MaxPrimitives = 0,
	//! The maximum number of parallel used streams
	MaxStreams,
	//! The maximum texture width
	MaxTextureWidth,
	//! The maximal texture height
	MaxTextureHeight,
	//! Textures must be a power of two
	TexturesPowerOfTwoOnly,
	//! Textures must be square
	TextureSquareOnly,
	//! Maximum number of parallel texture
	MaxSimultaneousTextures,
	//! The maximum number of lights.
	MaxLights,
	//! The maximum anisotropic filter value.
	MaxAnisotropy,
	//! The maximal number of simultaniousRenderTargets
	MaxSimultaneousRT,

	EDriverCaps_Count
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_EDRIVER_CAPS_H