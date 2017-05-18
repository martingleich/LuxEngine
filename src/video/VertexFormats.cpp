#include "video/VertexFormats.h"
#include "video/VertexTypes.h"
#include <cstddef>

namespace lux
{
namespace video
{

static VertexDeclaration VertexDeclaration_STANDARD;
static VertexDeclaration VertexDeclaration_TRANSFORMED;
static VertexDeclaration VertexDeclaration_TWO_TEXTURE;
static VertexDeclaration VertexDeclaration_TANGENTS;
static VertexDeclaration VertexDeclaration_TEXTURE_3D;
static VertexDeclaration VertexDeclaration_STANDARD_2D;

struct VertexDeclInitType
{
	VertexDeclInitType()
	{
		VertexDeclaration_STANDARD.AddElement(VertexElement::EUsage::Position, VertexElement::EType::Float3);
		VertexDeclaration_STANDARD.AddElement(VertexElement::EUsage::Normal, VertexElement::EType::Float3);
		VertexDeclaration_STANDARD.AddElement(VertexElement::EUsage::Diffuse, VertexElement::EType::Color);
		VertexDeclaration_STANDARD.AddElement(VertexElement::EUsage::Texcoord0, VertexElement::EType::Float2);
		lxAssert(VertexDeclaration_STANDARD.IsValid());
		static_assert(offsetof(video::Vertex3D, position) == 0, "Bad offset");
		static_assert(offsetof(video::Vertex3D, normal) == 12, "Bad offset");
		static_assert(offsetof(video::Vertex3D, color) == 24, "Bad offset");
		static_assert(offsetof(video::Vertex3D, texture) == 28, "Bad offset");
		static_assert(sizeof(video::Vertex3D) == 36, "Bad size");

		VertexDeclaration_TRANSFORMED.AddElement(VertexElement::EUsage::PositionNT, VertexElement::EType::Float4);
		VertexDeclaration_TRANSFORMED.AddElement(VertexElement::EUsage::Diffuse, VertexElement::EType::Color);
		VertexDeclaration_TRANSFORMED.AddElement(VertexElement::EUsage::Texcoord0, VertexElement::EType::Float2);
		lxAssert(VertexDeclaration_TRANSFORMED.IsValid());
		static_assert(offsetof(video::VertexTransformed, position) == 0, "Bad offset");
		static_assert(offsetof(video::VertexTransformed, RHW) == 12, "Bad offset");
		static_assert(offsetof(video::VertexTransformed, color) == 16, "Bad offset");
		static_assert(offsetof(video::VertexTransformed, texture) == 20, "Bad offset");
		static_assert(sizeof(video::VertexTransformed) == 28, "Bad size");

		VertexDeclaration_TWO_TEXTURE.AddElement(VertexElement::EUsage::Position, VertexElement::EType::Float3);
		VertexDeclaration_TWO_TEXTURE.AddElement(VertexElement::EUsage::Normal, VertexElement::EType::Float3);
		VertexDeclaration_TWO_TEXTURE.AddElement(VertexElement::EUsage::Diffuse, VertexElement::EType::Color);
		VertexDeclaration_TWO_TEXTURE.AddElement(VertexElement::EUsage::Texcoord0, VertexElement::EType::Float2);
		VertexDeclaration_TWO_TEXTURE.AddElement(VertexElement::EUsage::Texcoord1, VertexElement::EType::Float2);
		lxAssert(VertexDeclaration_TWO_TEXTURE.IsValid());
		static_assert(offsetof(video::Vertex2TCoords, position) == 0, "Bad offset");
		static_assert(offsetof(video::Vertex2TCoords, normal) == 12, "Bad offset");
		static_assert(offsetof(video::Vertex2TCoords, color) == 24, "Bad offset");
		static_assert(offsetof(video::Vertex2TCoords, texture) == 28, "Bad offset");
		static_assert(offsetof(video::Vertex2TCoords, texture2) == 36, "Bad offset");
		static_assert(sizeof(video::Vertex2TCoords) == 44, "Bad size");

		VertexDeclaration_TANGENTS.AddElement(VertexElement::EUsage::Position, VertexElement::EType::Float3);
		VertexDeclaration_TANGENTS.AddElement(VertexElement::EUsage::Normal, VertexElement::EType::Float3);
		VertexDeclaration_TANGENTS.AddElement(VertexElement::EUsage::Diffuse, VertexElement::EType::Color);
		VertexDeclaration_TANGENTS.AddElement(VertexElement::EUsage::Texcoord0, VertexElement::EType::Float2);
		VertexDeclaration_TANGENTS.AddElement(VertexElement::EUsage::Binormal, VertexElement::EType::Float3);
		VertexDeclaration_TANGENTS.AddElement(VertexElement::EUsage::Tangent, VertexElement::EType::Float3);
		lxAssert(VertexDeclaration_TANGENTS.IsValid());
		static_assert(offsetof(video::VertexTangents, position) == 0, "Bad offset");
		static_assert(offsetof(video::VertexTangents, normal) == 12, "Bad offset");
		static_assert(offsetof(video::VertexTangents, color) == 24, "Bad offset");
		static_assert(offsetof(video::VertexTangents, texture) == 28, "Bad offset");
		static_assert(offsetof(video::VertexTangents, binormal) == 36, "Bad offset");
		static_assert(offsetof(video::VertexTangents, tangent) == 48, "Bad offset");
		static_assert(sizeof(video::VertexTangents) == 60, "Bad size");

		VertexDeclaration_TEXTURE_3D.AddElement(VertexElement::EUsage::Position, VertexElement::EType::Float3);
		VertexDeclaration_TEXTURE_3D.AddElement(VertexElement::EUsage::Texcoord0, VertexElement::EType::Float3);
		lxAssert(VertexDeclaration_TEXTURE_3D.IsValid());
		static_assert(offsetof(video::Vertex3DTCoord, position) == 0, "Bad offset");
		static_assert(offsetof(video::Vertex3DTCoord, texture) == 12, "Bad offset");
		static_assert(sizeof(video::Vertex3DTCoord) == 24, "Bad size");

		VertexDeclaration_STANDARD_2D.AddElement(VertexElement::EUsage::Position, VertexElement::EType::Float2);
		VertexDeclaration_STANDARD_2D.AddElement(VertexElement::EUsage::Diffuse, VertexElement::EType::Color);
		VertexDeclaration_STANDARD_2D.AddElement(VertexElement::EUsage::Texcoord0, VertexElement::EType::Float2);
		lxAssert(VertexDeclaration_STANDARD_2D.IsValid());
		static_assert(offsetof(video::Vertex2D, position) == 0, "Bad offset");
		static_assert(offsetof(video::Vertex2D, color) == 8, "Bad offset");
		static_assert(offsetof(video::Vertex2D, texture) == 12, "Bad offset");
		static_assert(sizeof(video::Vertex2D) == 20, "Bad size");
	}
};

static VertexDeclInitType VertexDeclInitInstance;

const VertexFormat VertexFormat::STANDARD("Standard", VertexDeclaration_STANDARD);
const VertexFormat VertexFormat::TRANSFORMED("Transformed", VertexDeclaration_TRANSFORMED);
const VertexFormat VertexFormat::TWO_TEXTURE("Two_Texture", VertexDeclaration_TWO_TEXTURE);
const VertexFormat VertexFormat::TANGENTS("Tangents", VertexDeclaration_TANGENTS);
const VertexFormat VertexFormat::TEXTURE_3D("Texture_3D", VertexDeclaration_TEXTURE_3D);
const VertexFormat VertexFormat::STANDARD_2D("Standard_2D", VertexDeclaration_STANDARD_2D);

}
}
