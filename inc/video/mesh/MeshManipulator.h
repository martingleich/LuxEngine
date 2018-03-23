#ifndef INCLUDED_LUX_MESH_MANIPULATOR_H
#define INCLUDED_LUX_MESH_MANIPULATOR_H
#include "video/mesh/VideoMesh.h"
#include "video/mesh/Geometry.h"
#include "video/VertexBuffer.h"
#include "video/IndexBuffer.h"
#include "video/Color.h"

#include "math/Matrix4.h"
#include "video/VideoDriver.h"

namespace lux
{
namespace video
{

//! An interface to transform and manipulate geometry.
/**
A mesh manipulator can be used to perform transformation on geometry or
meshes, like flipping surfaces, recalculation normals, unwelding meshes and
many more. It is intended to ease generation of geometry or fix broken imported
meshes, it not intended for runtime animations or the like.<br>
Each implementation
*/
class MeshManipulator
{
public:
	virtual ~MeshManipulator() {}

	//! Apply to a mesh.
	/**
	The transformation should be applied in-place, i.e. the returned pointer
	should be equal to the passed one. The geometry shouldn't be changed too,
	only vertex or indexbuffer may be created anew.
	*/
	virtual Mesh* operator()(Mesh* mesh) = 0;
	//! Apply to geometry.
	/**
	The transformation should be applied in-place, i.e. the returned pointer
	should be equal to the passed one.
	*/
	virtual Geometry* operator()(Geometry* geo) = 0;

protected:
	//! Helper method, get all modifiable positions in geometry.
	core::StrideRange<math::Vector3F> GetPositions(Geometry* geo)
	{
		return geo->GetVertices()->Elements
			<math::Vector3F>(VertexElement::EUsage::Position);
	}

	//! Helper method, get all modifiable normals in geometry.
	core::StrideRange<math::Vector3F> GetNormals(Geometry* geo)
	{
		return geo->GetVertices()->Elements
			<math::Vector3F>(VertexElement::EUsage::Normal);
	}

	//! Helper method, get all modifiable colors in geometry.
	core::StrideRange<Color> GetColors(Geometry* geo)
	{
		return geo->GetVertices()->Elements
			<Color>(VertexElement::EUsage::Diffuse);
	}

	//! Helper method, get all modifiable texture coords in geometry.
	core::StrideRange<math::Vector2F> GetTCoords(Geometry* geo, u32 index = 0)
	{
		return geo->GetVertices()->Elements
			<math::Vector2F>(VertexElement::TexcoordN(index));
	}
};

//! Flip the surfaces of all triangles.
/**
Has only a effect on triangle geometry.<br>
All normals are flipped, and the winding of all triangles is flipped.
*/
class MeshManipulatorFlipSurfaces : public MeshManipulator
{
public:
	Mesh* operator()(Mesh* mesh)
	{
		(*this)(mesh->GetGeometry());
		return mesh;
	}

	Geometry* operator()(Geometry* geo)
	{
		for(auto& n : GetNormals(geo))
			n = -n;

		geo->SetFrontFaceWinding(video::FlipWinding(geo->GetFrontFaceWinding()));
		geo->GetVertices()->Update();

		return geo;
	}
};

//! Applies a matrix to all positions and normals in the mesh.
/**
All positions are transformed with the matrix.
All normals are transformed with the inversed transposed matrix.
The bounding box is recalculated.
*/
class MeshManipulatorApplyMatrix : public MeshManipulator
{
public:
	//! Constructor
	/**
	\param transform The matrix to apply to the vertices.
	*/
	MeshManipulatorApplyMatrix(const math::Matrix4& transform) :
		m_Transform(transform),
		m_TransformIT(transform.GetTransformInverted().GetTransposed())
	{
	}
	//! Constructor
	/**
	\param transform The matrix to apply to the vertices.
	\param transformIT The inverse transposed matrix to apply to the vertices.
	*/
	MeshManipulatorApplyMatrix(
		const math::Matrix4& transform,
		const math::Matrix4& transformIT) :
		m_Transform(transform),
		m_TransformIT(transformIT)
	{
	}

	Mesh* operator()(Mesh* mesh)
	{
		(*this)(mesh->GetGeometry());
		mesh->RecalculateBoundingBox();
		return mesh;
	}

	Geometry* operator()(Geometry* geo)
	{
		math::AABBoxF newBoundingBox(INFINITY, INFINITY, INFINITY, -INFINITY, -INFINITY, -INFINITY);

		for(auto& p : GetPositions(geo)) {
			p = m_Transform.TransformVector(p);
			newBoundingBox.AddPoint(p);
		}
		for(auto& n : GetNormals(geo)) {
			n = m_TransformIT.TransformVector(n);
		}

		geo->SetBoundingBox(newBoundingBox);
		geo->GetVertices()->Update();

		return geo;
	}

private:
	math::Matrix4 m_Transform;
	math::Matrix4 m_TransformIT;
};

//! Move each vertex along its normal vector.
/**
The bounding box is recalucated.
*/
class MeshManipulatorScaleAnlongNormals : public MeshManipulator
{
public:
	//! Constructor
	/**
	\param scale The distance each point is moved.
	*/
	MeshManipulatorScaleAnlongNormals(float scale = 1.0f) :
		m_Scale(scale)
	{
	}

	Mesh* operator()(Mesh* mesh)
	{
		return (*this)(mesh, m_Scale);
	}

	//! Operator with custom scale.
	Mesh* operator()(Mesh* mesh, float scale)
	{
		(*this)(mesh->GetGeometry(), scale);
		mesh->RecalculateBoundingBox();
		return mesh;
	}

	Geometry* operator()(Geometry* geo)
	{
		return (*this)(geo, m_Scale);
	}

	//! Operator with custom scale.
	Geometry* operator()(Geometry* geo, float scale)
	{
		math::AABBoxF newBoundingBox(INFINITY, INFINITY, INFINITY, -INFINITY, -INFINITY, -INFINITY);

		auto pos = GetPositions(geo);
		auto nor = GetNormals(geo);
		if(pos.Size() == nor.Size()) {
			for(int i = 0; i < pos.Size(); ++i) {
				pos[i] += scale * nor[i];
				newBoundingBox.AddPoint(pos[i]);
			}
		}

		geo->SetBoundingBox(newBoundingBox);
		geo->GetVertices()->Update();

		return geo;
	}

private:
	float m_Scale;
};

//! Unwelds a mesh.
/**
After this operation each vertex is only part of a single primitive.
This is done by duplicating each vertex which is part of multiple primitives.
*/
class MeshManipulatorUnweld : public MeshManipulator
{
public:
	Mesh* operator()(Mesh* mesh)
	{
		(*this)(mesh->GetGeometry());
		return mesh;
	}

	Geometry* operator()(Geometry* geo)
	{
		return (*this)(geo, nullptr);
	}

	//! Operator which doesn't work inplace, and receives a seperated target geometry.
	Geometry* operator()(Geometry* geo, Geometry* targetGeo)
	{
		if(targetGeo == nullptr)
			targetGeo = geo;

		auto primType = geo->GetPrimitiveType();
		if(primType == EPrimitiveType::Points)
			return geo;

		auto indices = geo->GetIndices();
		auto vertices = geo->GetVertices();
		auto primCount = geo->GetPrimitiveCount();
		auto indexType = EIndexFormat::Bit16;
		if(primCount > math::Constants<u16>::max())
			indexType = EIndexFormat::Bit32;

		auto targetIndices = targetGeo->GetIndices();
		auto targetVertices = targetGeo->GetVertices();
		auto bufferMgr =
			VideoDriver::Instance()->GetBufferManager();
		if(targetGeo == geo) {
			targetIndices = bufferMgr->CreateIndexBuffer();
			targetVertices = bufferMgr->CreateVertexBuffer();
		}

		if(primType == EPrimitiveType::Lines ||
			primType == EPrimitiveType::LineStrip) {
			throw core::ErrorException("Unweld LineStrip not implemented");
		}

		if(primType == EPrimitiveType::TriangleStrip ||
			primType == EPrimitiveType::Triangles ||
			primType == EPrimitiveType::TriangleFan) {
			// Create new vertices and indices, if necessary.
			targetIndices->SetFormat(indexType, false);
			targetIndices->SetSize(primCount * 3, false);
			targetVertices->SetSize(primCount * 3, false);
			targetVertices->SetFormat(vertices->GetFormat(), vertices->GetStream());

			auto strideV = vertices->GetStride();
			auto srcV = (u8*)vertices->Pointer_c(0, vertices->GetSize());
			auto dstV = (u8*)targetVertices->Pointer(0, targetVertices->GetSize());
			auto dstI = (u8*)targetIndices->Pointer(0, targetIndices->GetSize());
			bool winding = geo->GetFrontFaceWinding() == video::EFaceWinding::CCW;
			for(int i = 0; i < primCount; ++i) {
				int a, b, c;
				switch(primType) {
				case EPrimitiveType::TriangleStrip:
					if(winding) {
						a = indices->GetIndex(3 * i);
						b = indices->GetIndex(3 * i + 1);
						c = indices->GetIndex(3 * i + 2);
					} else {
						a = indices->GetIndex(3 * i + 1);
						b = indices->GetIndex(3 * i);
						c = indices->GetIndex(3 * i + 2);
					}
					break;
				case EPrimitiveType::TriangleFan:
					a = indices->GetIndex(0);
					b = indices->GetIndex(3 * i);
					c = indices->GetIndex(3 * i + 1);
					break;
				case EPrimitiveType::Triangles:
					a = indices->GetIndex(3 * i);
					b = indices->GetIndex(3 * i + 1);
					c = indices->GetIndex(3 * i + 2);
					break;
				default:
					a = b = c = 0;
				}
				int index = 3 * i + 0;
				memcpy(dstV + index*strideV, srcV + a*strideV, strideV);
				index = 3 * i + 1;
				memcpy(dstV + index*strideV, srcV + b*strideV, strideV);
				index = 3 * i + 2;
				memcpy(dstV + index*strideV, srcV + c*strideV, strideV);
			}

			targetVertices->Update();

			if(targetIndices->GetFormat() == EIndexFormat::Bit32) {
				for(u32 i = 0; i < 3 * (u32)primCount; ++i)
					memcpy(dstI + i * 4, &i, 4);
			} else {
				for(u16 i = 0; i < 3 * (u16)primCount; ++i)
					memcpy(dstI + i * 2, &i, 2);
			}
			targetIndices->Update();
		}

		targetGeo->SetIndices(targetIndices);
		targetGeo->SetVertices(targetVertices);
		return targetGeo;
	}
};

//! Recalculates the normals of the mesh.
/**
Generates flat-shading i.e. The normal of each vertex is equal to the
face it belongs to.<br>
All faces a vertex belongs to, should have the same normal. This is the
case if the mesh in fully unwelded.<br>
If the vertexbuffer of the passed mesh doesn't contains normals, this
manipulator does _nothing_.
*/
class MeshManipulatorRecalculateNormals : public MeshManipulator
{
public:
	Mesh* operator()(Mesh* mesh)
	{
		(*this)(mesh->GetGeometry());
		return mesh;
	}

	Geometry* operator()(Geometry* geo)
	{
		auto indices = geo->GetIndices();
		auto pos = GetPositions(geo);
		auto nor = GetNormals(geo);
		auto winding = geo->GetFrontFaceWinding() == video::EFaceWinding::CCW;
		int ob = winding ? 1 : 2;
		int oc = winding ? 2 : 1;
		for(int i = 0; i < geo->GetPrimitiveCount(); ++i) {
			int a = indices->GetIndex(3 * i);
			int b = indices->GetIndex(3 * i + ob);
			int c = indices->GetIndex(3 * i + oc);

			auto v1 = pos[b] - pos[a];
			auto v2 = pos[c] - pos[a];
			nor[a] = nor[b] = nor[c] = v1.Cross(v2).Normal();
		}

		geo->GetVertices()->Update();
		return geo;
	}
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_LUX_MESH_MANIPULATOR_H