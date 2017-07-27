#include "GeometryCreatorCube.h"
#include "video/VideoDriver.h"
#include "video/IndexBuffer.h"
#include "video/VertexBuffer.h"
#include "video/VertexTypes.h"
#include "video/mesh/Geometry.h"

namespace lux
{
namespace video
{

static void Swap(u16& a, u16& b)
{
	u16 t = a;
	a = b;
	b = t;
}

GeometryCreatorCube::GeometryCreatorCube()
{
	m_Package.AddParam("size", math::Vector3F(1.0f, 1.0f, 1.0f));
	m_Package.AddParam("tes", math::vector3i(2, 2, 2));
	m_Package.AddParam("tex", math::Vector3F(1.0f, 1.0f, 1.0f));
	m_Package.AddParam("inside", false);
}

const String& GeometryCreatorCube::GetName() const
{
	static const String name = "cube";
	return name;
}

const core::ParamPackage& GeometryCreatorCube::GetParams() const
{
	return m_Package;
}

StrongRef<Geometry> GeometryCreatorCube::CreateGeometry(const core::PackagePuffer& params)
{
	const math::Vector3F size = params.FromID(0, true);
	const math::vector3i tes = params.FromID(1, true);
	const math::Vector3F tex = params.FromID(2, true);
	const bool inside = params.FromID(3, true);

	return CreateGeometry(
		size.x, size.y, size.z,
		tes.x, tes.y, tes.z,
		tex.x, tex.y, tex.z,
		inside);
}

StrongRef<Geometry> GeometryCreatorCube::CreateGeometry(
	float sizeX, float sizeY, float sizeZ,
	s32 tesX, s32 tesY, s32 tesZ,
	float texX, float texY, float texZ,
	bool inside)
{
	if(sizeX <= 0.0f || sizeY <= 0.0f || sizeZ <= 0.0f)
		throw core::InvalidArgumentException("sizeX, sizeY, sizeZ", "Must be bigger than zero");

	if(tesX < 2 || tesY < 2 || tesZ < 2)
		throw core::InvalidArgumentException("tesX, tesY, tesZ", "Must be bigger than 1");

	const u32 vertexCount = tesX*tesY * 2 + tesX*tesZ * 2 + tesZ*tesY * 2;
	if(vertexCount > 0xFFFF) // 16 Bit indices.
		throw core::InvalidArgumentException("Too many indices");

	const u32 indexCount =
		6 * ((tesX - 1)*(tesY - 1) * 2 +
		(tesX - 1)*(tesZ - 1) * 2 +
		(tesY - 1)*(tesZ - 1) * 2);

	const math::Vector3F tex(texX, texY, texZ);
	const math::vector3i tes(tesX, tesY, tesZ);
	const math::Vector3F size(sizeX, sizeY, sizeZ);

	StrongRef<Geometry> subMesh = VideoDriver::Instance()->CreateGeometry(
		VertexFormat::STANDARD, EHardwareBufferMapping::Static, vertexCount,
		EIndexFormat::Bit16, EHardwareBufferMapping::Static, indexCount,
		EPrimitiveType::Triangles);

	StrongRef<VertexBuffer> vertexBuffer = subMesh->GetVertices();

	StrongRef<IndexBuffer> indexBuffer = subMesh->GetIndices();

	static const struct
	{
		u32 a, b;
	} faces[] = {{0, 1}, {0, 2}, {1, 2}};

	for(u32 f = 0; f < 6; ++f) {
		const bool side = (f >= 3);

		const u32 a = faces[f % 3].a;
		const u32 b = faces[f % 3].b;
		const u32 c = 3 - a - b;

		const u32 ta = (u32)tes[a];
		const u32 tb = (u32)tes[b];

		const float sa = size[a];
		const float sb = size[b];
		const float sc = size[c];

		const u32 firstIndex = vertexBuffer->GetCursor();

		for(u32 j = 0; j < tb; ++j) {
			for(u32 i = 0; i < ta; ++i) {
				Vertex3D v;

				v.position[a] = ((i * sa) / (ta - 1)) - sa / 2;
				v.position[b] = ((j * sb) / (tb - 1)) - sb / 2;
				v.position[c] = (side ? sc : -sc) / 2;
				v.normal[a] = 0.0f;
				v.normal[b] = 0.0f;
				v.normal[c] = (side ^ inside) ? 1.0f : -1.0f;
				v.texture.x = (i * tex[a]) / (ta - 1);
				v.texture.y = (j * tex[b]) / (tb - 1);

				v.color = video::Color::White;

				vertexBuffer->AddVertex(&v);
			}
		}

		for(u32 i = 0; i < ta - 1; ++i) {
			for(u32 j = 0; j < tb - 1; ++j) {
				u16 indices[6];
				indices[0] = (u16)((i + 0) + (j + 0)*ta + firstIndex);
				indices[1] = (u16)((i + 0) + (j + 1)*ta + firstIndex);
				indices[2] = (u16)((i + 1) + (j + 0)*ta + firstIndex);

				indices[3] = (u16)((i + 0) + (j + 1)*ta + firstIndex);
				indices[4] = (u16)((i + 1) + (j + 1)*ta + firstIndex);
				indices[5] = (u16)((i + 1) + (j + 0)*ta + firstIndex);

				if((f % 2 == 1) ^ inside) {
					Swap(indices[1], indices[2]);
					Swap(indices[4], indices[5]);
				}

				indexBuffer->AddIndices(indices, 6);
			}
		}
	}

	indexBuffer->Update();
	vertexBuffer->Update();

	subMesh->SetBoundingBox(math::AABBoxF(
		-sizeX / 2, -sizeY / 2, -sizeZ / 2,
		sizeX / 2, sizeY / 2, sizeZ / 2));

	return subMesh;
}

}
}
