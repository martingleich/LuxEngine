#include "GeometryCreatorArrow.h"
#include "video/VideoDriver.h"
#include "video/IndexBuffer.h"
#include "video/VertexBuffer.h"
#include "video/VertexTypes.h"
#include "video/SubMesh.h"

#define GEO_CREATOR_ARROW_SPLIT_POINT

namespace lux
{
namespace scene
{

GeometryCreatorArrow::GeometryCreatorArrow()
{
	m_Package.AddParam("shaft_height", 2.0f);
	m_Package.AddParam("head_heigth", 1.0f);
	m_Package.AddParam("shaft_radius", 0.5f);
	m_Package.AddParam("head_radius", 0.75f);
	m_Package.AddParam("sectors", 8);
}

const string& GeometryCreatorArrow::GetName() const
{
	static const string name = "arrow";
	return name;
}

const core::ParamPackage& GeometryCreatorArrow::GetParams() const
{
	return m_Package;
}

StrongRef<video::SubMesh> GeometryCreatorArrow::CreateSubMesh(video::VideoDriver* driver, const core::PackagePuffer& params)
{
	LX_CHECK_NULL_ARG(driver);

	const float shaft_height = params.Param(0).Default(2.0f);
	const float head_height = params.Param(1).Default(1.0f);
	const float shaft_radius = params.Param(2).Default(0.5f);
	const float head_radius = params.Param(3).Default(0.75f);
	const s32 sectors = params.Param(4).Default(8);

	return CreateSubMesh(driver,
		shaft_height, head_height,
		shaft_radius, head_radius,
		sectors);
}

StrongRef<video::SubMesh> GeometryCreatorArrow::CreateSubMesh(video::VideoDriver* driver,
	float shaft_height, float head_height,
	float shaft_radius, float head_radius,
	s32 sectors)
{
	LX_CHECK_NULL_ARG(driver);

	if(shaft_height <= 0.0f || head_height <= 0.0f || shaft_radius <= 0.0f || head_radius <= 0.0f)
		throw core::InvalidArgumentException("shaft_height, head_height, shaft_radius, head_radius", "Must be bigger than zero");

	if(head_radius < shaft_radius)
		throw core::InvalidArgumentException("head_radius, shaft_radius", "Head radius must be bigger than shaft_radius");

	if(sectors < 3)
		throw core::InvalidArgumentException("sectors", "Number of sectors must be bigger than 2");

	// 6 Rings + 1 Arrowpoint
	const u32 vertexCount = sectors * 6 + 1;
	if(vertexCount > 0xFFFF) // 16 Bit indices.
		throw core::InvalidArgumentException("sectors", "Too many sectors");

	// Circle + Pipe  + Ring  + Head
	// sec-2  + sec*2 + sec*2 + sec Triangles
	const u32 indexCount = 3 * (6 * sectors - 2);
	StrongRef<video::SubMesh> subMesh = driver->CreateSubMesh(
		video::VertexFormat::STANDARD, video::EHardwareBufferMapping::Static, vertexCount,
		video::EIndexFormat::Bit16, video::EHardwareBufferMapping::Static, indexCount,
		video::EPrimitiveType::Triangles);

	StrongRef<video::VertexBuffer> vertexBuffer = subMesh->GetVertices();
	StrongRef<video::IndexBuffer> indexBuffer = subMesh->GetIndices();

	const float shaft_circum = 2.0f*shaft_radius*math::Constants<float>::pi();

	struct SinCos
	{
		float s;
		float c;
	};

	core::array<SinCos> sinCos;
	sinCos.Reserve(sectors);
	for(s32 i = 0; i <= sectors; ++i) {
		math::anglef a = i * (math::anglef::FULL / sectors);
		SinCos sc;
		sc.c = math::Cos(a);
		sc.s = math::Sin(a);
		sinCos.PushBack(sc);
	}

	////////////////////////////////////////////////////////////////////
	// Base Circle
	u32 baseIndex = 0;

	for(s32 i = 0; i <= sectors; ++i) {
		video::Vertex3D v;

		v.position.x = shaft_radius * sinCos[i].s;
		v.position.y = shaft_radius * sinCos[i].c;
		v.position.z = 0.0f;

		v.normal.x = 0.0f;
		v.normal.y = 0.0f;
		v.normal.z = -1.0f;

		v.texture.x = math::Constants<float>::reciprocal_pi() * sinCos[i].s / 2 + 0.5f;
		v.texture.y = math::Constants<float>::reciprocal_pi() * sinCos[i].c / 2 + 0.5f;

		vertexBuffer->AddVertex(&v);
	}

	for(s32 i = 0; i < sectors - 2; ++i) {
		u16 indices[3] = {
			(u16)0,
			(u16)(i + 1),
			(u16)(i + 2)};

		indexBuffer->AddIndices(indices, 3);
	}

	////////////////////////////////////////////////////////////////////
	// Pipe
	baseIndex = vertexBuffer->GetCursor();
	for(s32 f = 0; f < 2; ++f) {
		for(s32 i = 0; i <= sectors; ++i) {
			video::Vertex3D v;

			v.normal.x = sinCos[i].s;
			v.normal.y = sinCos[i].c;
			v.normal.z = 0.0f;

			v.position.x = shaft_radius * v.normal.x;
			v.position.y = shaft_radius * v.normal.y;
			v.position.z = (f == 0) ? 0.0f : shaft_height;

			v.texture.y = (f == 0) ? 0.0f : shaft_height / shaft_circum;
			v.texture.x = (float)i / sectors;

			vertexBuffer->AddVertex(&v);
		}
	}

	for(s32 i = 0; i < sectors; ++i) {
		u16 indices[6] = {
			(u16)(baseIndex + i),
			(u16)(baseIndex + i + sectors + 1),
			(u16)(baseIndex + i + 1),
			(u16)(baseIndex + i + 1),
			(u16)(baseIndex + i + sectors + 1),
			(u16)(baseIndex + i + sectors + 2)};

		indexBuffer->AddIndices(indices, 6);
	}

	////////////////////////////////////////////////////////////////////
	// Ring
	baseIndex = vertexBuffer->GetCursor();

	for(s32 f = 0; f < 2; ++f) {
		for(s32 i = 0; i <= sectors; ++i) {
			video::Vertex3D v;

			const float radius = (f == 0) ? shaft_radius : head_radius;
			v.position.x = radius * sinCos[i].s;
			v.position.y = radius * sinCos[i].c;
			v.position.z = shaft_height;

			v.normal.x = 0.0f;
			v.normal.y = 0.0f;
			v.normal.z = -1.0f;

			if(f == 0) {
				v.texture.x = math::Constants<float>::reciprocal_pi() * sinCos[i].s / 2 + 0.5f;
				v.texture.y = math::Constants<float>::reciprocal_pi() * sinCos[i].c / 2 + 0.5f;
			} else {
				v.texture.x = (head_radius / shaft_circum) * sinCos[i].s + 0.5f;
				v.texture.y = (head_radius / shaft_circum) * sinCos[i].c + 0.5f;
			}

			vertexBuffer->AddVertex(&v);
		}
	}

	for(s32 i = 0; i < sectors; ++i) {
		u16 indices[6] = {
			(u16)(baseIndex + i),
			(u16)(baseIndex + i + sectors + 1),
			(u16)(baseIndex + i + 1),
			(u16)(baseIndex + i + 1),
			(u16)(baseIndex + i + sectors + 1),
			(u16)(baseIndex + i + sectors + 2)};

		indexBuffer->AddIndices(indices, 6);
	}


	////////////////////////////////////////////////////////////////////
	// Cone
	baseIndex = vertexBuffer->GetCursor();

	const float head_s = sqrt(head_radius*head_radius + head_height*head_height);
	const math::anglef unroll_sector_angle = (head_radius / head_s) * math::anglef::FULL;
	for(s32 i = 0; i <= sectors; ++i) {
		video::Vertex3D v;

		const float s = sinCos[i].s;
		const float c = sinCos[i].c;

		v.position.x = head_radius * s;
		v.position.y = head_radius * c;
		v.position.z = shaft_height;

		v.normal.x = s;
		v.normal.y = c;
		v.normal.z = head_radius / head_height;
		v.normal /= sqrt(1 + v.normal.z*v.normal.z);

		const math::anglef sector_angle = unroll_sector_angle * ((float)i / sectors);
		v.texture.x = (head_s / shaft_circum) * math::Sin(sector_angle) + 0.5f;
		v.texture.y = (head_s / shaft_circum) * math::Cos(sector_angle) + 0.5f;

		vertexBuffer->AddVertex(&v);
	}

	for(s32 i = 0; i <= sectors; ++i) {
		video::Vertex3D v;
		v.position.x = 0.0f;
		v.position.y = 0.0f;
		v.position.z = shaft_height + head_height;

		v.normal.x = sinCos[i].s;
		v.normal.y = sinCos[i].c;
		v.normal.z = head_radius / head_height;

		v.texture.x = 0.5f;
		v.texture.y = 0.5f;

		v.normal /= sqrt(1 + v.normal.z*v.normal.z);

		vertexBuffer->AddVertex(&v);
	}

	for(s32 i = 0; i < sectors; ++i) {
		u16 indices[3] = {
			(u16)(baseIndex + i),
			(u16)(baseIndex + sectors + 1 + i),
			(u16)(baseIndex + i + 1)};

		indexBuffer->AddIndices(indices, 3);
	}

	indexBuffer->Update();
	vertexBuffer->Update();

	subMesh->SetBoundingBox(math::aabbox3df(
		-head_radius, -head_radius, 0.0f,
		head_radius, head_radius, shaft_height + head_height));

	return subMesh;
}

}
}
