#include "GeometryCreatorSphereUV.h"
#include "video/mesh/Geometry.h"
#include "video/IndexBuffer.h"
#include "video/VertexBuffer.h"
#include "video/VideoDriver.h"
#include "video/VertexTypes.h"

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

GeometryCreatorSphereUV::GeometryCreatorSphereUV()
{
	m_Params.AddParam("radius", 1.0f);
	m_Params.AddParam("rings", 16);
	m_Params.AddParam("sectors", 32);
	m_Params.AddParam("tex", math::vector2f(1, 1));
	m_Params.AddParam("inside", false);
}

const string& GeometryCreatorSphereUV::GetName() const
{
	static const string name = "sphereUV";
	return name;
}

const core::ParamPackage& GeometryCreatorSphereUV::GetParams() const
{
	return m_Params;
}

StrongRef<Geometry> GeometryCreatorSphereUV::CreateGeometry(const core::PackagePuffer& params)
{
	const float radius = params.Param(0);
	const s32 rings = params.Param(1);
	const s32 sectors = params.Param(2);
	const math::vector2f tex = params.Param(3);
	const bool inside = params.Param(4);

	return CreateGeometry(radius, rings, sectors, tex.x, tex.y, inside);
}

StrongRef<Geometry> GeometryCreatorSphereUV::CreateGeometry(
	float radius,
	s32 rings, s32 sectors, float texX, float texY,
	bool inside)
{
	if(radius <= 0.0f)
		throw core::InvalidArgumentException("radius", "Must be bigger than zero");

	if(rings < 2)
		throw core::InvalidArgumentException("rings", "Must be bigger than 1");

	if(sectors < 3)
		throw core::InvalidArgumentException("sectors", "Must be bigger than 2");

	const u32 vertexCount = (rings - 1) * (sectors + 1) + 2;
	const u32 indexCount = 6 * sectors * (rings - 1);

	if(vertexCount > 0xFFFF)
		throw core::InvalidArgumentException("Too many arguments");

	StrongRef<Geometry> subMesh = VideoDriver::Instance()->CreateGeometry(
		VertexFormat::STANDARD, EHardwareBufferMapping::Static, vertexCount,
		EIndexFormat::Bit16, EHardwareBufferMapping::Static, indexCount,
		EPrimitiveType::Triangles);

	StrongRef<VertexBuffer> vertexBuffer = subMesh->GetVertices();
	StrongRef<IndexBuffer> indexBuffer = subMesh->GetIndices();

	const s32 temp = rings-1;
	Vertex3D Vertex;
	u16 indices[6];
	math::anglef alpha;
	math::anglef beta;
	Vertex.texture = math::vector2f(0.0f);
	for(s32 x = 0; x <= sectors; ++x) {
		alpha = x * (math::anglef::FULL / sectors);

		for(s32 y = 0; y < temp; ++y) {
			beta = math::anglef::Radian(float(math::Constants<float>::pi() / rings)*(y - ((rings - 2)*0.5f)));

			Vertex.normal = math::vector3f::BuildFromPolar(alpha, beta, 1.0f);
			Vertex.position = Vertex.normal * radius;
			Vertex.texture.x = texX*(alpha/math::anglef::FULL);
			Vertex.texture.y = texY*(0.5f - beta/math::anglef::HALF);

			Vertex.color = Color::White;

			vertexBuffer->AddVertex(&Vertex);
		}
	}

	Vertex.position = math::vector3f(0.0f, radius, 0.0f);
	Vertex.normal = math::vector3f(0.0f, 1.0f, 0.0f);
	Vertex.texture = math::vector2f(0.5f*texX, 0.0f);
	vertexBuffer->AddVertex(&Vertex);

	Vertex.position *= -1.0f;
	Vertex.normal *= -1.0f;
	Vertex.texture = math::vector2f(0.5f*texX, texY);
	vertexBuffer->AddVertex(&Vertex);

	for(s32 quad = 0; quad < sectors; ++quad) {
		indices[0] = (u16)((rings - 1) * (sectors + 1) + 1);
		indices[1] = (u16)(temp * (quad + 1));
		indices[2] = (u16)(temp * quad);

		indices[3] = (u16)(indices[0] - 1);
		indices[4] = (u16)(indices[1] - 1);
		indices[5] = (u16)(indices[4] + temp);

		if(inside) {
			Swap(indices[1], indices[2]);
			Swap(indices[4], indices[5]);
		}

		indexBuffer->AddIndices(indices, 6);

		for(s32 i = 0; i < rings - 2; i++) {
			indices[0] = (u16)(temp * (quad + 1) + i);
			indices[1] = (u16)(temp * quad + 1 + i);
			indices[2] = (u16)(indices[1] - 1);

			indices[3] = (u16)(indices[0] + 1);
			indices[4] = indices[1];
			indices[5] = indices[0];

			if(inside) {
				Swap(indices[1], indices[2]);
				Swap(indices[4], indices[5]);
			}

			indexBuffer->AddIndices(indices, 6);
		}
	}

	vertexBuffer->Update();
	indexBuffer->Update();

	subMesh->SetBoundingBox(math::aabbox3df(-radius, -radius, -radius, radius, radius, radius));

	return subMesh;
}

}
}