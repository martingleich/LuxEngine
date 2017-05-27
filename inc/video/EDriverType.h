#ifndef INCLUDED_EDRIVER_TYPE_H
#define INCLUDED_EDRIVER_TYPE_H

namespace lux
{
namespace video
{

//! Diffrent types of usable video drivers
enum class EDriverType
{
	//! The direct3D 9 video driver
	Direct3D9 = 0,

	//! The null driver
	Null,
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_EDRIVER_TYPE_H