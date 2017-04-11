#include "GeometryCreatorSphereUV.h"
#include "video/SubMesh.h"
#include "video/IndexBuffer.h"
#include "video/VertexBuffer.h"
#include "video/VideoDriver.h"
#include "video/VertexTypes.h"

namespace lux
{
namespace scene
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
	m_Params.AddParam("segments", 32);
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

StrongRef<video::SubMesh> GeometryCreatorSphereUV::CreateSubMesh(video::VideoDriver* driver, const core::PackagePuffer& params)
{
	const float radius = params.Param(0);
	const s32 rings = params.Param(1);
	const s32 segments = params.Param(2);
	const math::vector2f tex = params.Param(3);
	const bool inside = params.Param(4);

	return CreateSubMesh(driver, radius, rings, segments, tex.x, tex.y, inside);
}

StrongRef<video::SubMesh> GeometryCreatorSphereUV::CreateSubMesh(video::VideoDriver* driver,
	float radius,
	s32 rings, s32 segments, float texX, float texY,
	bool inside)
{
	if(radius <= 0.0f)
		return nullptr;
	if(rings < 2)
		return nullptr;
	if(segments < 3)
		return nullptr;

	const u32 vertexCount = (rings - 1) * (segments + 1) + 2;
	const u32 indexCount = 6 * segments * (rings - 1);

	if(vertexCount > 0xFFFF)
		return nullptr;

	StrongRef<video::SubMesh> subMesh = driver->CreateSubMesh(
		video::VertexFormat::STANDARD, video::EHardwareBufferMapping::Static, vertexCount,
		video::EIndexFormat::Bit16, video::EHardwareBufferMapping::Static, indexCount,
		video::EPT_TRIANGLES);

	if(!subMesh)
		return nullptr;

	StrongRef<video::VertexBuffer> vertexBuffer = subMesh->GetVertices();
	if(!vertexBuffer)
		return nullptr;

	StrongRef<video::IndexBuffer> indexBuffer = subMesh->GetIndices();
	if(!indexBuffer)
		return nullptr;

	const s32 temp = rings-1;
	video::Vertex3D Vertex;
	u16 indices[6];
	math::anglef alpha;
	math::anglef beta;
	Vertex.texture = math::vector2f(0.0f);
	for(s32 x = 0; x <= segments; ++x) {
		alpha = x * (math::anglef::FULL / segments);

		for(s32 y = 0; y < temp; ++y) {
			beta = math::anglef::Radian(float(math::Constants<float>::pi() / rings)*(y - ((rings - 2)*0.5f)));

			Vertex.normal = math::vector3f::BuildFromPolar(alpha, beta, 1.0f);
			Vertex.position = Vertex.normal * radius;
			Vertex.texture.x = texX * (alpha / math::anglef::HALF);
			Vertex.texture.y = texY * ((beta / math::anglef::HALF+0.5f)*math::Constants<float>::half_pi());

			Vertex.color = video::Color::White;

			vertexBuffer->AddVertex(&Vertex);
		}
	}

	Vertex.position = math::vector3f(0.0f, radius, 0.0f);
	Vertex.normal = math::vector3f(0.0f, 1.0f, 0.0f);
	Vertex.texture = math::vector2f(0.5f*texX, texY*math::Constants<float>::half_pi());
	vertexBuffer->AddVertex(&Vertex);

	Vertex.position *= -1.0f;
	Vertex.normal *= -1.0f;
	Vertex.texture = math::vector2f(0.5f*texX, 0.0f);
	vertexBuffer->AddVertex(&Vertex);

	for(s32 quad = 0; quad < segments; ++quad) {
		indices[0] = (u16)((rings - 1) * (segments + 1) + 1);
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