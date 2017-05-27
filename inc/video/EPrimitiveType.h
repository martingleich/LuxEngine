#ifndef INCLUDED_EPRIMITIVE_TYPE_H
#define INCLUDED_EPRIMITIVE_TYPE_H

namespace lux
{
namespace video
{

//! Different Primitive Types
enum class EPrimitiveType
{
	//! One point per vertex
	Points,

	//! One line through all vertices
	LineStrip,

	//! Two following vertices, make a line
	Lines,

	//! After the first two vertices, every one creates a new triangle,
	//! the two older ones and the the create a triangle
	TriangleStrip,

	//! A triangle fan, first point in center
	TriangleFan,

	//! Three vertices create a triangle
	Triangles,
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_EPRIMITIVE_TYPE_H