#include "video/VertexFormats.h"

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

		VertexDeclaration_TRANSFORMED.AddElement(VertexElement::EUsage::PositionNT, VertexElement::EType::Float4);
		VertexDeclaration_TRANSFORMED.AddElement(VertexElement::EUsage::Diffuse, VertexElement::EType::Color);
		VertexDeclaration_TRANSFORMED.AddElement(VertexElement::EUsage::Texcoord0, VertexElement::EType::Float2);
		lxAssert(VertexDeclaration_TRANSFORMED.IsValid());

		VertexDeclaration_TWO_TEXTURE.AddElement(VertexElement::EUsage::Position, VertexElement::EType::Float3);
		VertexDeclaration_TWO_TEXTURE.AddElement(VertexElement::EUsage::Normal, VertexElement::EType::Float3);
		VertexDeclaration_TWO_TEXTURE.AddElement(VertexElement::EUsage::Diffuse, VertexElement::EType::Color);
		VertexDeclaration_TWO_TEXTURE.AddElement(VertexElement::EUsage::Texcoord0, VertexElement::EType::Float2);
		VertexDeclaration_TWO_TEXTURE.AddElement(VertexElement::EUsage::Texcoord1, VertexElement::EType::Float2);
		lxAssert(VertexDeclaration_TWO_TEXTURE.IsValid());

		VertexDeclaration_TANGENTS.AddElement(VertexElement::EUsage::Position, VertexElement::EType::Float3);
		VertexDeclaration_TANGENTS.AddElement(VertexElement::EUsage::Normal, VertexElement::EType::Float3);
		VertexDeclaration_TANGENTS.AddElement(VertexElement::EUsage::Diffuse, VertexElement::EType::Color);
		VertexDeclaration_TANGENTS.AddElement(VertexElement::EUsage::Texcoord0, VertexElement::EType::Float2);
		VertexDeclaration_TANGENTS.AddElement(VertexElement::EUsage::Binormal, VertexElement::EType::Float3);
		VertexDeclaration_TANGENTS.AddElement(VertexElement::EUsage::Tangent, VertexElement::EType::Float3);
		lxAssert(VertexDeclaration_TANGENTS.IsValid());

		VertexDeclaration_TEXTURE_3D.AddElement(VertexElement::EUsage::Position, VertexElement::EType::Float3);
		VertexDeclaration_TEXTURE_3D.AddElement(VertexElement::EUsage::Texcoord0, VertexElement::EType::Float3);
		lxAssert(VertexDeclaration_TEXTURE_3D.IsValid());

		VertexDeclaration_STANDARD_2D.AddElement(VertexElement::EUsage::Position, VertexElement::EType::Float2);
		VertexDeclaration_STANDARD_2D.AddElement(VertexElement::EUsage::Diffuse, VertexElement::EType::Color);
		VertexDeclaration_STANDARD_2D.AddElement(VertexElement::EUsage::Texcoord0, VertexElement::EType::Float2);
		lxAssert(VertexDeclaration_STANDARD_2D.IsValid());
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
