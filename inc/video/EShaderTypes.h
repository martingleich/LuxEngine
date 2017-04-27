#ifndef INCLUDED_ESHADERTYPES_H
#define INCLUDED_ESHADERTYPES_H

namespace lux
{
namespace video
{

enum class EShaderLanguage
{
	HLSL,
	Unknown
};

enum class EShaderType
{
	Vertex,
	Pixel,
	Unknown
};

}
}

#endif // !INCLUDED_ESHADERTYPES_H
