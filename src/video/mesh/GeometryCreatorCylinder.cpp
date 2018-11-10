#include "video/mesh/GeometryCreatorCylinder.h"

#include "video/VideoDriver.h"
#include "video/IndexBuffer.h"
#include "video/VertexBuffer.h"
#include "video/VertexTypes.h"
#include "video/mesh/Geometry.h"

namespace lux
{
namespace video
{
GeometryCreatorCylinder::GeometryCreatorCylinder()
{
	core::ParamPackageBuilder ppb;
	ppb.AddParam("radius", 0.5f);
	ppb.AddParam("height", 1.0f);
	ppb.AddParam("sectors", 16);
	ppb.AddParam("planes", 2);
	ppb.AddParam("tex", math::Vector2I(1, 1));
	ppb.AddParam("inside", false);
	m_Package = ppb.BuildAndReset();
}

StrongRef<Geometry> GeometryCreatorCylinder::CreateGeometry(const core::PackagePuffer& params)
{
	float radius = params.Param(0);
	float height = params.Param(1);
	s32 sectors = params.Param(2);
	s32 planes = params.Param(3);
	math::Vector2I tex = params.Param(4);
	bool inside = params.Param(5);

	return CreateGeometry(radius, height, sectors, planes, tex.x, tex.y, inside);

}

const core::String& GeometryCreatorCylinder::GetName() const
{
	static core::String name = "cylinder";
	return name;
}

const core::ParamPackage& GeometryCreatorCylinder::GetParams() const
{
	return m_Package;
}

StrongRef<Geometry> GeometryCreatorCylinder::CreateGeometry(
	float radius,
	float height,
	s32 sectors,
	s32 planes,
	s32 texX,
	s32 texY,
	bool inside)
{
	if(radius <= 0.0f || height <= 0.0f)
		throw core::GenericInvalidArgumentException("radius, height", "Must be bigger than zero");

	if(sectors < 3)
		throw core::GenericInvalidArgumentException("sectors", "Number of sectors must be bigger than 2");

	if(planes < 2)
		throw core::GenericInvalidArgumentException("planes", "Number of planes must be bigger than 1");

	const u32 vertexCount = 2 * (sectors + 1) + planes * (sectors + 1);
	if(vertexCount > 0xFFFF) // 16 Bit indices.
		throw core::GenericInvalidArgumentException("sectors, planes", "Too many sectors or planes");

	const u32 indexCount = 3 * (2 * sectors + 2 * (planes - 1)*sectors);
	StrongRef<Geometry> geo = VideoDriver::Instance()->CreateGeometry(
		VertexFormat::STANDARD, EHardwareBufferMapping::Static, vertexCount,
		EIndexFormat::Bit16, EHardwareBufferMapping::Static, indexCount,
		EPrimitiveType::Triangles);

	StrongRef<VertexBuffer> vertexBuffer = geo->GetVertices();
	StrongRef<IndexBuffer> indexBuffer = geo->GetIndices();

	Vertex3D vertex;
	vertex.color = Color::White;

	// Generate walls
	u16 curId = 0;
	for(s32 i = 0; i < planes; ++i) {
		float y = height / (planes - 1)*i;
		y = y - height / 2;
		for(s32 j = 0; j < sectors + 1; ++j) {
			math::AngleF alpha = (math::AngleF::FULL / sectors) * j;
			math::Vector3F off = math::Vector3F(
				math::Sin(alpha),
				0.0f,
				math::Cos(alpha));

			vertex.position = radius * off + math::Vector3F(0.0f, y, 0.0f);
			vertex.normal = inside ? -off : off;
			vertex.texture.x = texX * (alpha / math::AngleF::FULL);
			vertex.texture.y = texY * ((float)i / (planes - 1));

			vertexBuffer->AddVertex(&vertex);

			if(j < sectors && i < planes - 1) {
				u16 indices[6] = {
					curId, (u16)(curId + 1), (u16)(curId + sectors + 1),
					(u16)(curId + sectors + 1), (u16)(curId + 1), (u16)(curId + sectors + 2)
				};

				if(inside) {
					u16 t = indices[0];
					indices[0] = indices[1];
					indices[1] = t;
					t = indices[3];
					indices[3] = indices[4];
					indices[4] = t;
				}

				indexBuffer->AddIndices(indices, 6);
			}

			++curId;
		}
	}

	// Top and bottom rings
	for(s32 i = 0; i < 2; ++i) {
		float y = height * i - height / 2;
		float ny = i == 0 ? -1.0f : 1.0f;

		u16 baseId = (u16)vertexBuffer->GetCursor();
		curId = baseId;
		u16 centerId = (u16)(curId + sectors);
		for(s32 j = 0; j < sectors; ++j) {
			math::AngleF alpha = (math::AngleF::FULL / sectors) * j;
			math::Vector3F off = math::Vector3F(
				math::Sin(alpha),
				0.0f,
				math::Cos(alpha));

			vertex.position = radius * off + math::Vector3F(0.0f, y, 0.0f);
			vertex.normal = math::Vector3F(0.0f, ny, 0.0f);
			vertex.texture.x = texX * (0.5f*off.x + 0.5f);
			vertex.texture.y = texY * (0.5f*off.z + 0.5f);

			vertexBuffer->AddVertex(&vertex);

			u16 indices[3];

			// Topside ccw, Button cw, the other way if inside
			u16 next = curId + 1;
			if(next >= baseId + sectors)
				next = baseId;
			if((ny == -1.0f) != inside) {
				indices[0] = next;
				indices[1] = curId;
				indices[2] = centerId;
			} else {
				indices[0] = curId;
				indices[1] = next;
				indices[2] = centerId;
			}

			indexBuffer->AddIndices(indices, 3);
			++curId;
		}

		vertex.position = math::Vector3F(0.0f, height / 2 * ny, 0.0f);
		vertex.normal = math::Vector3F(0.0f, ny, 0.0f);
		vertex.texture = math::Vector2F(0.5f, 0.5f);
		vertexBuffer->AddVertex(&vertex);
	}

	vertexBuffer->Update();
	indexBuffer->Update();

	geo->SetBoundingBox(math::AABBoxF(
		-radius, -height / 2, -radius,
		radius, height / 2, radius));

	return geo;
}

} // namespace lux
} // namespace video
