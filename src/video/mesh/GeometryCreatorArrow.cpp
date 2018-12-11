#include "GeometryCreatorArrow.h"
#include "video/VideoDriver.h"
#include "video/IndexBuffer.h"
#include "video/VertexBuffer.h"
#include "video/VertexTypes.h"
#include "video/mesh/Geometry.h"

#define GEO_CREATOR_ARROW_SPLIT_POINT

namespace lux
{
namespace video
{

GeometryCreatorArrow::GeometryCreatorArrow()
{
	core::ParamPackageBuilder ppb;
	ppb.AddParam("shaftHeight", 2.0f);
	ppb.AddParam("headHeigth", 1.0f);
	ppb.AddParam("shaftRadius", 0.5f);
	ppb.AddParam("headRadius", 0.75f);
	ppb.AddParam("sectors", 8);
	m_Package = ppb.BuildAndReset();
}

const core::String& GeometryCreatorArrow::GetName() const
{
	static const core::String name = "arrow";
	return name;
}

const core::ParamPackage& GeometryCreatorArrow::GetParams() const
{
	return m_Package;
}

StrongRef<Geometry> GeometryCreatorArrow::CreateGeometry(const core::PackagePuffer& params)
{
	const float shaft_height = params.Param(0).GetDefault(2.0f);
	const float head_height = params.Param(1).GetDefault(1.0f);
	const float shaft_radius = params.Param(2).GetDefault(0.5f);
	const float head_radius = params.Param(3).GetDefault(0.75f);
	const s32 sectors = params.Param(4).GetDefault(8);

	return CreateGeometry(
		shaft_height, head_height,
		shaft_radius, head_radius,
		sectors);
}

StrongRef<Geometry> GeometryCreatorArrow::CreateGeometry(
	float shaft_height, float head_height,
	float shaft_radius, float head_radius,
	s32 sectors)
{
	if(shaft_height <= 0.0f || head_height <= 0.0f || shaft_radius <= 0.0f || head_radius <= 0.0f)
		throw core::GenericInvalidArgumentException("shaftHeight, headHeight, shaftRadius, head_radius", "Must be bigger than zero");

	if(head_radius < shaft_radius)
		throw core::GenericInvalidArgumentException("headRadius, shaftRadius", "Head radius must be bigger than shaft_radius");

	if(sectors < 3)
		throw core::GenericInvalidArgumentException("sectors", "Number of sectors must be bigger than 2");

	// 6 Rings + 1 Arrowpoint
	const u32 vertexCount = sectors * 6 + 1;
	if(vertexCount > 0xFFFF) // 16 Bit indices.
		throw core::GenericInvalidArgumentException("sectors", "Too many sectors");

	// Circle + Pipe  + Ring  + Head
	// sec-2  + sec*2 + sec*2 + sec Triangles
	const u32 indexCount = 3 * (6 * sectors - 2);
	StrongRef<Geometry> subMesh = VideoDriver::Instance()->CreateGeometry(
		VertexFormat::STANDARD, EHardwareBufferMapping::Static, vertexCount,
		EIndexFormat::Bit16, EHardwareBufferMapping::Static, indexCount,
		EPrimitiveType::Triangles);

	StrongRef<VertexBuffer> vertexBuffer = subMesh->GetVertices();
	StrongRef<IndexBuffer> indexBuffer = subMesh->GetIndices();

	const float shaft_circum = 2.0f*shaft_radius*math::Constants<float>::pi();

	struct SinCos
	{
		float s;
		float c;
	};

	core::Array<SinCos> sinCos;
	sinCos.Reserve(sectors);
	for(s32 i = 0; i <= sectors; ++i) {
		math::AngleF a = i * (math::AngleF::FULL / sectors);
		SinCos sc;
		sc.c = math::Cos(a);
		sc.s = math::Sin(a);
		sinCos.PushBack(sc);
	}

	////////////////////////////////////////////////////////////////////
	// Base Circle
	u32 baseIndex = 0;

	for(s32 i = 0; i <= sectors; ++i) {
		Vertex3D v;

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
			Vertex3D v;

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
			Vertex3D v;

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

	const float head_s = std::sqrt(head_radius*head_radius + head_height*head_height);
	const math::AngleF unroll_sector_angle = (head_radius / head_s) * math::AngleF::FULL;
	for(s32 i = 0; i <= sectors; ++i) {
		Vertex3D v;

		const float s = sinCos[i].s;
		const float c = sinCos[i].c;

		v.position.x = head_radius * s;
		v.position.y = head_radius * c;
		v.position.z = shaft_height;

		v.normal.x = s;
		v.normal.y = c;
		v.normal.z = head_radius / head_height;
		v.normal /= std::sqrt(1 + v.normal.z*v.normal.z);

		const math::AngleF sector_angle = unroll_sector_angle * ((float)i / sectors);
		v.texture.x = (head_s / shaft_circum) * math::Sin(sector_angle) + 0.5f;
		v.texture.y = (head_s / shaft_circum) * math::Cos(sector_angle) + 0.5f;

		vertexBuffer->AddVertex(&v);
	}

	for(s32 i = 0; i <= sectors; ++i) {
		Vertex3D v;
		v.position.x = 0.0f;
		v.position.y = 0.0f;
		v.position.z = shaft_height + head_height;

		v.normal.x = sinCos[i].s;
		v.normal.y = sinCos[i].c;
		v.normal.z = head_radius / head_height;

		v.texture.x = 0.5f;
		v.texture.y = 0.5f;

		v.normal /= std::sqrt(1 + v.normal.z*v.normal.z);

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

	subMesh->SetBoundingBox(math::AABBoxF(
		-head_radius, -head_radius, 0.0f,
		head_radius, head_radius, shaft_height + head_height));

	return subMesh;
}

}
}
