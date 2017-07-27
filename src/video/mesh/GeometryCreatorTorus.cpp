#include "video/mesh/GeometryCreatorTorus.h"

#include "video/VideoDriver.h"
#include "video/IndexBuffer.h"
#include "video/VertexBuffer.h"
#include "video/VertexTypes.h"
#include "video/mesh/Geometry.h"

namespace lux
{
namespace video
{

GeometryCreatorTorus::GeometryCreatorTorus()
{
	m_Package.AddParam("radius_major", 1.0f);
	m_Package.AddParam("radius_minor", 0.5f);
	m_Package.AddParam("sectors_major", 48);
	m_Package.AddParam("sectors_minor", 12);
	m_Package.AddParam("tex", math::Vector2I(1, 1));
	m_Package.AddParam("inside", false);
}

StrongRef<Geometry> GeometryCreatorTorus::CreateGeometry(const core::PackagePuffer& params)
{
	float radiusMajor = params.Param(0);
	float radiusMinor = params.Param(1);
	s32 sectorsMajor = params.Param(2);
	s32 sectorsMinor = params.Param(3);
	math::Vector2I tex = params.Param(4);
	bool inside = params.Param(5);

	return CreateGeometry(radiusMajor, radiusMinor, sectorsMajor, sectorsMinor, tex.x, tex.y, inside);

}

const String& GeometryCreatorTorus::GetName() const
{
	static String name = "torus";
	return name;
}

const core::ParamPackage& GeometryCreatorTorus::GetParams() const
{
	return m_Package;
}

StrongRef<Geometry> GeometryCreatorTorus::CreateGeometry(
	float radiusMajor,
	float radiusMinor,
	s32 sectorsMajor,
	s32 sectorsMinor,
	s32 texX,
	s32 texY,
	bool inside)
{
	if(radiusMajor <= 0.0f || radiusMinor <= 0.0f)
		throw core::InvalidArgumentException("radiusMajor, radiusMinor", "Must be bigger than zero");

	if(sectorsMajor < 3 || sectorsMinor < 3)
		throw core::InvalidArgumentException("sectorsMajor, sectorsMinor", "Number of sectors must be bigger than 2");

	const u32 vertexCount = (sectorsMinor + 1)*(sectorsMajor + 1);
	if(vertexCount > 0xFFFF) // 16 Bit indices.
		throw core::InvalidArgumentException("sectorsMajor, sectorsMinor", "Too many sectors");

	// two triangles per sector
	const u32 indexCount = sectorsMinor*sectorsMajor * 2 * 3;
	StrongRef<Geometry> geo = VideoDriver::Instance()->CreateGeometry(
		VertexFormat::STANDARD, EHardwareBufferMapping::Static, vertexCount,
		EIndexFormat::Bit16, EHardwareBufferMapping::Static, indexCount,
		EPrimitiveType::Triangles);

	StrongRef<VertexBuffer> vertexBuffer = geo->GetVertices();
	StrongRef<IndexBuffer> indexBuffer = geo->GetIndices();

	Vertex3D vertex;
	vertex.color = Color::White;
	u16 curId = 0;
	for(s32 i = 0; i < sectorsMajor + 1; ++i) {
		math::AngleF alpha = (math::AngleF::FULL / sectorsMajor) * i;
		math::Vector3F base(
			radiusMajor*math::Sin(alpha),
			0.0f,
			radiusMajor*math::Cos(alpha));
		for(s32 j = 0; j < sectorsMinor + 1; ++j) {
			math::AngleF beta = (math::AngleF::FULL / sectorsMinor) * j;
			math::Vector3F off = math::Vector3F::BuildFromPolar(
				alpha, beta, 1.0f);

			vertex.position = base + radiusMinor*off;
			vertex.normal = inside ? -off : off;
			vertex.texture.x = texX * (alpha / math::AngleF::FULL);
			vertex.texture.y = texY * (beta / math::AngleF::FULL);

			vertexBuffer->AddVertex(&vertex);

			if(j < sectorsMinor && i < sectorsMajor) {
				u16 indices[6] = {
					curId, (u16)(curId + sectorsMinor + 1), (u16)(curId + 1),
					(u16)(curId + 1), (u16)(curId + sectorsMinor + 1), (u16)(curId + sectorsMinor + 2)
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

	vertexBuffer->Update();
	indexBuffer->Update();

	float r = radiusMajor + radiusMinor;
	geo->SetBoundingBox(math::AABBoxF(
		-r, -radiusMinor, -r, r, radiusMinor, r));

	return geo;
}

}
}