#include "GeometryCreatorPlane.h"
#include "video/SubMesh.h"
#include "video/VertexBuffer.h"
#include "video/IndexBuffer.h"
#include "video/VideoDriver.h"
#include "video/VertexTypes.h"


namespace lux
{
namespace scene
{

GeometryCreatorPlane::GeometryCreatorPlane()
{
	m_Params.AddParam("size", math::vector2f(1.0f, 1.0f));
	m_Params.AddParam("tes", math::vector2i(1, 1));
	m_Params.AddParam("tex", math::vector2f(1, 1));
}

const core::ParamPackage& GeometryCreatorPlane::GetParams() const
{
	return m_Params;
}

const string& GeometryCreatorPlane::GetName() const
{
	static const string name = "plane";
	return name;
}

StrongRef<video::SubMesh> GeometryCreatorPlane::CreateSubMesh(video::VideoDriver* driver, const core::PackagePuffer& params)
{
	const math::vector2f size = params.FromID(0, true);
	const math::vector2i tesselation = params.FromID(1, true);
	const math::vector2f textureRepeat = params.FromID(2, true);

	return CreateSubMesh(driver, size.x, size.y, tesselation.x, tesselation.y, textureRepeat.x, textureRepeat.y, nullptr, nullptr);
}

StrongRef<video::SubMesh> GeometryCreatorPlane::CreateSubMesh(
	video::VideoDriver* driver,
	float sizeX, float sizeY,
	s32 tesX, s32 tesY,
	float texX, float texY,
	float(*function)(void* ctx, float x, float y), void* context)
{
	LX_CHECK_NULL_ARG(driver);

	if(sizeX <= 0.0f || sizeY <= 0.0f)
		throw core::InvalidArgumentException("sizeX, sizeY", "Must be bigger than zero");

	if(tesX <= 1 || tesY <= 1)
		throw core::InvalidArgumentException("tesX, tesY", "Must be bigger than 1");

	const u32 vertexCount = tesX * tesY;
	const u32 indexCount = (tesX - 1)*(tesY - 1) * 6;

	if(vertexCount > 0xFFFF)
		throw core::InvalidArgumentException("Too many arguments");

	auto GetVertexIndex = [=](s32 x, s32 y) -> u16 { return (u16)(y * tesX + x); };

	StrongRef<video::SubMesh> subMesh = driver->CreateSubMesh(
		video::VertexFormat::STANDARD, video::EHardwareBufferMapping::Static, vertexCount,
		video::EIndexFormat::Bit16, video::EHardwareBufferMapping::Static, indexCount,
		video::EPrimitiveType::Triangles);

	StrongRef<video::VertexBuffer> vertexBuffer = subMesh->GetVertices();
	StrongRef<video::IndexBuffer> indexBuffer = subMesh->GetIndices();

	math::aabbox3df boundingBox;
	video::Vertex3D vertex;
	for(s32 x = 0; x < tesX; ++x) {
		for(s32 y = 0; y < tesY; ++y) {
			vertex.color = video::Color::White;
			vertex.position = math::vector3f(
				((float)(x) / (float)(tesX - 1) - 0.5f) * sizeX,
				0.0f,
				((float)(-y) / (float)(tesX - 1) + 0.5f) * sizeY);

			if(function)
				vertex.position.y = function(context, vertex.position.x, vertex.position.z);

			vertex.texture = math::vector2f((float)(x) / (tesX-1) * texX, (float)(y) / (tesY-1) * texY);
			vertexBuffer->SetVertex(&vertex, GetVertexIndex(x, y));

			boundingBox.AddPoint(vertex.position);

			if(x < tesX - 1 && y < tesY - 1) {
				u16 Indices[6] = {GetVertexIndex(x, y),
					GetVertexIndex(x + 1, y),
					GetVertexIndex(x, y + 1),
					GetVertexIndex(x, y + 1),
					GetVertexIndex(x + 1, y),
					GetVertexIndex(x + 1, y + 1)};

				indexBuffer->AddIndices(Indices, 6);
			}
		}
	}

	for(s32 x = 0; x < tesX; ++x) {
		for(s32 y = 0; y < tesY; ++y) {
			if(function) {
				math::vector3f h1, h2;
				math::vector3f v1, v2;
				math::vector3f c;

				c = ((video::Vertex3D*)vertexBuffer->Pointer(GetVertexIndex(x, y), 1))->position;
				if(x + 1 < tesX)
					h1 = ((video::Vertex3D*)vertexBuffer->Pointer(GetVertexIndex(x + 1, y), 1))->position;
				else
					h1 = c;
				if(x - 1 > 0)
					h2 = ((video::Vertex3D*)vertexBuffer->Pointer(GetVertexIndex(x - 1, y), 1))->position;
				else
					h2 = c;
				if(y + 1 < tesY)
					v1 = ((video::Vertex3D*)vertexBuffer->Pointer(GetVertexIndex(x, y + 1), 1))->position;
				else
					v1 = c;
				if(y - 1 > 0)
					v2 = ((video::Vertex3D*)vertexBuffer->Pointer(GetVertexIndex(x, y - 1), 1))->position;
				else
					v2 = c;

				math::vector3f tan = ((c - h1) + (h2 - c));
				math::vector3f bin = ((c - v1) + (v2 - c));

				math::vector3f normal = tan.Cross(bin).Normalize();
				((video::Vertex3D*)vertexBuffer->Pointer(GetVertexIndex(x, y), 1))->normal = normal;
			} else {
				((video::Vertex3D*)vertexBuffer->Pointer(GetVertexIndex(x, y), 1))->normal = math::vector3f::UNIT_Y;
			}
		}
	}

	indexBuffer->Update();
	vertexBuffer->Update();

	subMesh->SetBoundingBox(boundingBox);

	return subMesh;
}

}
}