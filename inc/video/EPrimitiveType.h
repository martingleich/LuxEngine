#ifndef INCLUDED_EPRIMITIVE_TYPE_H
#define INCLUDED_EPRIMITIVE_TYPE_H

namespace lux
{
namespace video
{

//! Different Primitive Types
enum EPrimitiveType
{
	//! One point per vertex
	EPT_POINTS = 0,

	//! One line through all vertices
	EPT_LINE_STRIP,

	//! Two following vertices, make a line
	EPT_LINES,

	//! After the first two vertices, every one creates a new triangle,
	//! the two older ones and the the create a triangle
	EPT_TRIANGLE_STRIP,

	//! A triangle fan, first point in center
	EPT_TRIANGLE_FAN,

	//! Three vertices create a triangle
	EPT_TRIANGLES,

	//! Single vertices become small quads
	EPT_POINT_SPRITES
};

}
}

#endif // #ifndef INCLUDED_EPRIMITIVE_TYPE_H