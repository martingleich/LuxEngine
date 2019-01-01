#include "GeometryCreatorPlane.h"
#include "video/mesh/Geometry.h"
#include "video/VertexBuffer.h"
#include "video/IndexBuffer.h"
#include "video/VideoDriver.h"
#include "video/VertexTypes.h"


namespace lux
{
namespace video
{

GeometryCreatorPlane::GeometryCreatorPlane()
{
	core::ParamPackageBuilder ppb;
	ppb.AddParam("size", math::Vector2F(1.0f, 1.0f));
	ppb.AddParam("tes", math::Vector2I(1, 1));
	ppb.AddParam("tex", math::Vector2F(1, 1));
	m_Params = ppb.BuildAndReset();
}

const core::ParamPackage& GeometryCreatorPlane::GetParams() const
{
	return m_Params;
}

StrongRef<Geometry> GeometryCreatorPlane::CreateGeometry(const core::PackagePuffer& params)
{
	const math::Vector2F size = params.FromID(0, true).Get<math::Vector2F>();
	const math::Vector2I tesselation = params.FromID(1, true).Get<math::Vector2I>();
	const math::Vector2F textureRepeat = params.FromID(2, true).Get<math::Vector2F>();

	return CreateGeometry(size.x, size.y, tesselation.x, tesselation.y, textureRepeat.x, textureRepeat.y, nullptr, nullptr);
}

StrongRef<Geometry> GeometryCreatorPlane::CreateGeometry(
	float sizeX, float sizeY,
	s32 tesX, s32 tesY,
	float texX, float texY,
	float(*function)(void* ctx, float x, float y), void* context)
{
	if(sizeX <= 0.0f || sizeY <= 0.0f)
		throw core::GenericInvalidArgumentException("sizeX, sizeY", "Must be bigger than zero");

	if(tesX <= 1 || tesY <= 1)
		throw core::GenericInvalidArgumentException("tesX, tesY", "Must be bigger than 1");

	const u32 vertexCount = tesX * tesY;
	const u32 indexCount = (tesX - 1)*(tesY - 1) * 6;
	const bool use16Bit = vertexCount <= 0xFFFF;

	auto GetVertexIndex = [=](u32 x, u32 y) -> u32 { return (u32)(y * tesX + x); };

	StrongRef<Geometry> subMesh = VideoDriver::Instance()->CreateGeometry(
		VertexFormat::STANDARD, EHardwareBufferMapping::Static, vertexCount,
		use16Bit ? EIndexFormat::Bit16 : EIndexFormat::Bit32, EHardwareBufferMapping::Static, indexCount,
		EPrimitiveType::Triangles);

	StrongRef<VertexBuffer> vertexBuffer = subMesh->GetVertices();
	StrongRef<IndexBuffer> indexBuffer = subMesh->GetIndices();

	math::AABBoxF boundingBox;
	video::Vertex3D vertex;
	for(u32 x = 0; x < (u32)tesX; ++x) {
		for(u32 y = 0; y < (u32)tesY; ++y) {
			vertex.color = Color::White;
			vertex.position = math::Vector3F(
				((float)(x) / (float)(tesX - 1) - 0.5f) * sizeX,
				0.0f,
				(-(float)(y) / (float)(tesY - 1) + 0.5f) * sizeY);

			if(function)
				vertex.position.y = function(context, vertex.position.x, vertex.position.z);

			vertex.texture = math::Vector2F((float)(x) / (tesX - 1) * texX, (float)(y) / (tesY - 1) * texY);
			vertexBuffer->SetVertex(&vertex, GetVertexIndex(x, y));

			boundingBox.AddPoint(vertex.position);

			if(x < (u32)tesX - 1 && y < (u32)tesY - 1) {
				u32 Indices[6] = {
					GetVertexIndex(x, y),
					GetVertexIndex(x + 1, y),
					GetVertexIndex(x, y + 1),
					GetVertexIndex(x, y + 1),
					GetVertexIndex(x + 1, y),
					GetVertexIndex(x + 1, y + 1)};

				indexBuffer->AddIndices32(Indices, 6);
			}
		}
	}

	for(u32 x = 0; x < (u32)tesX; ++x) {
		for(u32 y = 0; y < (u32)tesY; ++y) {
			if(function) {
				math::Vector3F h1, h2;
				math::Vector3F v1, v2;
				math::Vector3F c;

				c = ((Vertex3D*)vertexBuffer->Pointer(GetVertexIndex(x, y), 1))->position;
				if(x + 1 < (u32)tesX)
					h1 = ((Vertex3D*)vertexBuffer->Pointer(GetVertexIndex(x + 1, y), 1))->position;
				else
					h1 = c;
				if(x > 1)
					h2 = ((Vertex3D*)vertexBuffer->Pointer(GetVertexIndex(x - 1, y), 1))->position;
				else
					h2 = c;
				if(y + 1 < (u32)tesY)
					v1 = ((Vertex3D*)vertexBuffer->Pointer(GetVertexIndex(x, y + 1), 1))->position;
				else
					v1 = c;
				if(y > 1)
					v2 = ((Vertex3D*)vertexBuffer->Pointer(GetVertexIndex(x, y - 1), 1))->position;
				else
					v2 = c;

				math::Vector3F tan = ((c - h1) + (h2 - c));
				math::Vector3F bin = ((c - v1) + (v2 - c));

				math::Vector3F normal = tan.Cross(bin).Normalize();
				((Vertex3D*)vertexBuffer->Pointer(GetVertexIndex(x, y), 1))->normal = normal;
			} else {
				((Vertex3D*)vertexBuffer->Pointer(GetVertexIndex(x, y), 1))->normal = math::Vector3F::UNIT_Y;
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