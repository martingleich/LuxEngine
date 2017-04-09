#ifndef INCLUDED_HARDWARE_BUFFER_CONSTANTS_H
#define INCLUDED_HARDWARE_BUFFER_CONSTANTS_H

namespace lux
{
namespace video
{

//! The type of the hardware buffer
enum class EHardwareBufferType
{
	Index = 0, //!< A index buffer
	Vertex, //!< A vertex buffer
};

//! How should data be saved in hardware
enum class EHardwareBufferMapping
{
	Never = 0,  //!< Dont save in hardware  
	Static,     //!< data isnt changed often, saved in static area
	Dynamic,    //!< data is high frequent, saved in dynamic area
};

//! The data saved in a index buffer
enum EIndexFormat
{
	Bit16, //!< 16 Bit per index
	Bit32, //!< 32 Bit per index
};

}
}


#endif // #ifndef INCLUDED_HARDWARE_BUFFER_CONSTANTS_H