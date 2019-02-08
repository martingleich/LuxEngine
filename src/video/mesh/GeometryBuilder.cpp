#include "video/mesh/GeometryBuilder.h"
#include "video/mesh/Geometry.h"
#include "video/VertexBuffer.h"
#include "video/IndexBuffer.h"
#include "video/VideoDriver.h"
#include "video/VertexTypes.h"

namespace lux
{
namespace video
{

GeometryBuilder::GeometryBuilder()
{
}

GeometryBuilder::~GeometryBuilder()
{
}

void GeometryBuilder::Reset()
{
	m_Geometry = nullptr;
}

StrongRef<Geometry> GeometryBuilder::GetPointer()
{
	return m_Geometry;
}

StrongRef<Geometry> GeometryBuilder::GetGeoPointer(int vcount, int icount)
{
	if(!m_Geometry) {
		m_Geometry = VideoDriver::Instance()->CreateGeometry(
		VertexFormat::STANDARD, EHardwareBufferMapping::Static, vcount,
		EIndexFormat::Bit16, EHardwareBufferMapping::Static, icount,
		EPrimitiveType::Triangles);
	} else {
		m_Geometry->GetVertices()->SetSize(vcount, false);
		m_Geometry->GetIndices()->SetSize(icount, false);
	}

	return m_Geometry;
}

StrongRef<Geometry> GeometryBuilder::Finalize()
{
	if(m_Geometry) {
		m_Geometry->GetVertices()->Update();
		m_Geometry->GetIndices()->Update();
	}
	auto geo = m_Geometry;
	m_Geometry = nullptr;
	return geo;
}

GeometryBuilder& GeometryBuilder::CreatePlane(
	float sizeX, float sizeY,
	s32 tesX, s32 tesY,
	float texX, float texY,
	float(*function)(void* ctx, float x, float y),
	void* context)
{
	if(sizeX <= 0.0f || sizeY <= 0.0f)
		throw core::GenericInvalidArgumentException("sizeX, sizeY", "Must be bigger than zero");

	if(tesX <= 1 || tesY <= 1)
		throw core::GenericInvalidArgumentException("tesX, tesY", "Must be bigger than 1");

	const u32 vertexCount = tesX * tesY;
	const u32 indexCount = (tesX - 1)*(tesY - 1) * 6;
	const bool use16Bit = vertexCount <= 0xFFFF;

	auto GetVertexIndex = [=](u32 x, u32 y) -> u32 { return (u32)(y * tesX + x); };

	StrongRef<Geometry> subMesh = GetGeoPointer(vertexCount, indexCount);

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

	subMesh->SetBoundingBox(boundingBox);

	return *this;
}

GeometryBuilder& GeometryBuilder::CreateUVSphere(
	float radius,
	s32 rings, s32 sectors,
	float texX, float texY,
	bool inside)
{
	if(radius <= 0.0f)
		throw core::GenericInvalidArgumentException("radius", "Must be bigger than zero");

	if(rings < 2)
		throw core::GenericInvalidArgumentException("rings", "Must be bigger than 1");

	if(sectors < 3)
		throw core::GenericInvalidArgumentException("sectors", "Must be bigger than 2");

	const u32 vertexCount = (rings - 1) * (sectors + 1) + 2;
	const u32 indexCount = 6 * sectors * (rings - 1);

	if(vertexCount > 0xFFFF)
		throw core::GenericInvalidArgumentException("sectors,rings", "Too many vertices.");

	StrongRef<Geometry> subMesh = GetGeoPointer(vertexCount, indexCount);

	StrongRef<VertexBuffer> vertexBuffer = subMesh->GetVertices();
	StrongRef<IndexBuffer> indexBuffer = subMesh->GetIndices();

	const s32 temp = rings - 1;
	Vertex3D Vertex;
	u16 indices[6];
	math::AngleF alpha;
	math::AngleF beta;
	Vertex.texture = math::Vector2F(0.0f);
	for(s32 x = 0; x <= sectors; ++x) {
		alpha = x * (math::AngleF::FULL / sectors);

		for(s32 y = 0; y < temp; ++y) {
			beta = (math::AngleF::HALF / rings) * (y - ((rings - 2)*0.5f));

			Vertex.normal = math::Vector3F::BuildFromPolar(alpha, beta, 1);
			Vertex.position = Vertex.normal * radius;
			Vertex.texture.x = texX*(alpha / math::AngleF::FULL);
			Vertex.texture.y = texY*(0.5f - beta / math::AngleF::HALF);

			Vertex.color = Color::White;

			vertexBuffer->AddVertex(&Vertex);
		}
	}

	Vertex.position = math::Vector3F(0.0f, radius, 0.0f);
	Vertex.normal = math::Vector3F(0.0f, 1.0f, 0.0f);
	Vertex.texture = math::Vector2F(0.5f*texX, 0.0f);
	vertexBuffer->AddVertex(&Vertex);

	Vertex.position *= -1.0f;
	Vertex.normal *= -1.0f;
	Vertex.texture = math::Vector2F(0.5f*texX, texY);
	vertexBuffer->AddVertex(&Vertex);

	for(s32 quad = 0; quad < sectors; ++quad) {
		indices[0] = (u16)((rings - 1) * (sectors + 1) + 1);
		indices[1] = (u16)(temp * (quad + 1));
		indices[2] = (u16)(temp * quad);

		indices[3] = (u16)(indices[0] - 1);
		indices[4] = (u16)(indices[1] - 1);
		indices[5] = (u16)(indices[4] + temp);

		if(inside) {
			std::swap(indices[1], indices[2]);
			std::swap(indices[4], indices[5]);
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
				std::swap(indices[1], indices[2]);
				std::swap(indices[4], indices[5]);
			}

			indexBuffer->AddIndices(indices, 6);
		}
	}

	subMesh->SetBoundingBox(math::AABBoxF(-radius, -radius, -radius, radius, radius, radius));

	return *this;
}

GeometryBuilder& GeometryBuilder::CreateCube(
	float sizeX, float sizeY, float sizeZ,
	s32 tesX, s32 tesY, s32 tesZ,
	float texX, float texY, float texZ,
	bool inside)
{
	if(sizeX <= 0.0f || sizeY <= 0.0f || sizeZ <= 0.0f)
		throw core::GenericInvalidArgumentException("sizeX, sizeY, sizeZ", "Must be bigger than zero");

	if(tesX < 2 || tesY < 2 || tesZ < 2)
		throw core::GenericInvalidArgumentException("tesX, tesY, tesZ", "Must be bigger than 1");

	const u32 vertexCount = tesX * tesY * 2 + tesX * tesZ * 2 + tesZ * tesY * 2;
	if(vertexCount > 0xFFFF) // 16 Bit indices.
		throw core::GenericInvalidArgumentException("tesX,tesY,tesZ", "Too many indices");

	const u32 indexCount =
		6 * ((tesX - 1)*(tesY - 1) * 2 +
		(tesX - 1)*(tesZ - 1) * 2 +
			(tesY - 1)*(tesZ - 1) * 2);

	const math::Vector3F tex(texX, texY, texZ);
	const math::Vector3I tes(tesX, tesY, tesZ);
	const math::Vector3F size(sizeX, sizeY, sizeZ);

	auto subMesh = GetGeoPointer(vertexCount, indexCount);
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
					std::swap(indices[1], indices[2]);
					std::swap(indices[4], indices[5]);
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

	return *this;
}

GeometryBuilder& GeometryBuilder::CreateArrowMesh(
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
	StrongRef<Geometry> subMesh = GetGeoPointer(vertexCount, indexCount);
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

	subMesh->SetBoundingBox(math::AABBoxF(
		-head_radius, -head_radius, 0.0f,
		head_radius, head_radius, shaft_height + head_height));

	return *this;
}

GeometryBuilder& GeometryBuilder::CreateCylinder(
	float radius, float height,
	s32 sectors, s32 planes,
	s32 texX, s32 texY,
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
	StrongRef<Geometry> geo = GetGeoPointer(vertexCount, indexCount);

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

	geo->SetBoundingBox(math::AABBoxF(
		-radius, -height / 2, -radius,
		radius, height / 2, radius));

	return *this;
}

GeometryBuilder& GeometryBuilder::CreateTorus(
	float radiusMajor, float radiusMinor,
	s32 sectorsMajor, s32 sectorsMinor,
	s32 texX, s32 texY,
	bool inside)
{
	if(radiusMajor <= 0.0f || radiusMinor <= 0.0f)
		throw core::GenericInvalidArgumentException("radiusMajor, radiusMinor", "Must be bigger than zero");

	if(sectorsMajor < 3 || sectorsMinor < 3)
		throw core::GenericInvalidArgumentException("sectorsMajor, sectorsMinor", "Number of sectors must be bigger than 2");

	const u32 vertexCount = (sectorsMinor + 1)*(sectorsMajor + 1);
	if(vertexCount > 0xFFFF) // 16 Bit indices.
		throw core::GenericInvalidArgumentException("sectorsMajor, sectorsMinor", "Too many sectors");

	// two triangles per sector
	const u32 indexCount = sectorsMinor*sectorsMajor * 2 * 3;
	StrongRef<Geometry> geo = GetGeoPointer(vertexCount, indexCount);
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

	float r = radiusMajor + radiusMinor;
	geo->SetBoundingBox(math::AABBoxF(
		-r, -radiusMinor, -r, r, radiusMinor, r));

	return *this;
}

} // namespace video
} // namespace lux