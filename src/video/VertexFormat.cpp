#include "video/VertexFormat.h"
#include "video/VertexTypes.h"
#include <cstddef>

namespace lux
{
namespace video
{

const VertexFormat VertexFormat::STANDARD("Standard", {
	{offsetof(video::Vertex3D, position), VertexElement::EType::Float3, VertexElement::EUsage::Position},
	{offsetof(video::Vertex3D, normal), VertexElement::EType::Float3, VertexElement::EUsage::Normal},
	{offsetof(video::Vertex3D, color), VertexElement::EType::Color, VertexElement::EUsage::Diffuse},
	{offsetof(video::Vertex3D, texture), VertexElement::EType::Float2, VertexElement::EUsage::Texcoord0}});
const VertexFormat VertexFormat::POS_ONLY("PosOnly", {
	{offsetof(video::VertexPosOnly, position), VertexElement::EType::Float3, VertexElement::EUsage::Position}});
const VertexFormat VertexFormat::POSW_ONLY("PosWOnly", {
	{offsetof(video::VertexPosOnly, position), VertexElement::EType::Float4, VertexElement::EUsage::PositionNT}});
const VertexFormat VertexFormat::TRANSFORMED("Transformed", {
	{offsetof(video::VertexTransformed, position), VertexElement::EType::Float4, VertexElement::EUsage::PositionNT},
	{offsetof(video::VertexTransformed, color), VertexElement::EType::Color, VertexElement::EUsage::Diffuse},
	{offsetof(video::VertexTransformed, texture), VertexElement::EType::Float2, VertexElement::EUsage::Texcoord0}});
const VertexFormat VertexFormat::TWO_TEXTURE("Two_Texture", {
	{offsetof(video::Vertex2TCoords, position), VertexElement::EType::Float3, VertexElement::EUsage::Position},
	{offsetof(video::Vertex2TCoords, normal), VertexElement::EType::Float3, VertexElement::EUsage::Normal},
	{offsetof(video::Vertex2TCoords, color), VertexElement::EType::Color, VertexElement::EUsage::Diffuse},
	{offsetof(video::Vertex2TCoords, texture), VertexElement::EType::Float2, VertexElement::EUsage::Texcoord0},
	{offsetof(video::Vertex2TCoords, texture2), VertexElement::EType::Float2, VertexElement::EUsage::Texcoord1}});
const VertexFormat VertexFormat::TANGENTS("Tangents", {
	{offsetof(video::VertexTangents, position), VertexElement::EType::Float3, VertexElement::EUsage::Position},
	{offsetof(video::VertexTangents, normal), VertexElement::EType::Float3, VertexElement::EUsage::Normal},
	{offsetof(video::VertexTangents, color), VertexElement::EType::Color, VertexElement::EUsage::Diffuse},
	{offsetof(video::VertexTangents, texture), VertexElement::EType::Float2, VertexElement::EUsage::Texcoord0},
	{offsetof(video::VertexTangents, tangent), VertexElement::EType::Float3, VertexElement::EUsage::Tangent},
	{offsetof(video::VertexTangents, binormal), VertexElement::EType::Float3, VertexElement::EUsage::Binormal}});
const VertexFormat VertexFormat::TEXTURE_3D("Texture_3D", {
	{offsetof(video::Vertex3DTCoord, position), VertexElement::EType::Float3, VertexElement::EUsage::Position},
	{offsetof(video::Vertex3DTCoord, texture), VertexElement::EType::Float3, VertexElement::EUsage::Texcoord0}});
const VertexFormat VertexFormat::STANDARD_2D("Standard_2D", {
	{offsetof(video::Vertex2D, position), VertexElement::EType::Float2, VertexElement::EUsage::Position},
	{offsetof(video::Vertex2D, color), VertexElement::EType::Color, VertexElement::EUsage::Diffuse},
	{offsetof(video::Vertex2D, texture), VertexElement::EType::Float2, VertexElement::EUsage::Texcoord0}});

static_assert(sizeof(video::Vertex3D) == 36, "Bad size");
static_assert(sizeof(video::VertexPosOnly) == 12, "Bad size");
static_assert(sizeof(video::VertexPosWOnly) == 16, "Bad size");
static_assert(sizeof(video::VertexTransformed) == 28, "Bad size");
static_assert(sizeof(video::Vertex2TCoords) == 44, "Bad size");
static_assert(sizeof(video::VertexTangents) == 60, "Bad size");
static_assert(sizeof(video::Vertex3DTCoord) == 24, "Bad size");
static_assert(sizeof(video::Vertex2D) == 20, "Bad size");

VertexFormat::VertexFormat()
{
	static StrongRef<SharedData> shared = LUX_NEW(SharedData)("", {}, 0);
	m_Data = shared;
}

}
}
