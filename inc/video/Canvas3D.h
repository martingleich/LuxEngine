#ifndef INCLUDED_VIDEO_CANVAS3D_H
#define INCLUDED_VIDEO_CANVAS3D_H
#include "core/ReferenceCounted.h"
#include "video/Color.h"
#include "video/Pass.h"
#include "video/VertexFormat.h"
#include "video/VertexTypes.h"
#include "math/Vector3.h"
#include "math/Matrix4.h"
#include "math/Transformation.h"
#include "math/CurveInterpolation.h"

namespace lux
{
namespace video
{
class Renderer;

class Pen3D
{
public:
	Pen3D() :
		m_Color(video::Color::White)
	{
	}

	Pen3D(video::Color c) :
		m_Color(c)
	{
	}

	Color GetColor() const { return m_Color; }
	void SetColor(Color c) { m_Color = c; }

private:
	Color m_Color;
};

class Brush3D
{
public:
	Brush3D() :
		m_Color(video::Color::White)
	{
	}
	Brush3D(Color c) :
		m_Color(c)
	{
	}

	Color GetColor() const { return m_Color; }
	void SetColor(Color c) { m_Color = c; }

private:
	Color m_Color;
};

class Canvas3DSystem : public ReferenceCounted
{
public:
	struct PenVertex
	{
		math::Vector3F pos;
		Color color;
		PenVertex() {}
		PenVertex(const math::Vector3F& p, Color c) :
			pos(p), color(c)
		{
		}
	};

public:
	LUX_API static void Initialize();
	LUX_API static void Destroy();
	LUX_API static Canvas3DSystem* Instance();

	LUX_API Canvas3DSystem();
	LUX_API ~Canvas3DSystem();

	LUX_API const Pass& GetPenPass() const;
	LUX_API const Pass& GetBrushPass() const;
	LUX_API const VertexFormat& GetPenVertexFormat() const;
	LUX_API const VertexFormat& GetBrushVertexFormat() const;

private:
	Pass m_PenPass;
	VertexFormat m_PenFormat;

	Pass m_BrushPass;
};

class Canvas3D
{
	enum EPass
	{
		PenPass,
		BrushPass,
		NonePass
	};

public:
	LUX_API Canvas3D(const math::Matrix4& transform = math::Matrix4::IDENTITY, float polyOffset = 0.0f, video::Renderer* renderer = nullptr);

	Canvas3D(const Canvas3D& other) = delete;
	Canvas3D& operator=(const Canvas3D& other) = delete;

	LUX_API ~Canvas3D();

	inline void SelectPen(const Pen3D& pen) { m_Pen = pen; }
	inline void SelectPen(Color color) { SelectPen(Pen3D(color)); }

	inline void SelectBrush(const Brush3D& brush) { m_Brush = brush; }
	inline void SelectBrush(Color color) { SelectBrush(Brush3D(color)); }

	inline const Pen3D& GetPen() const { return m_Pen; }
	inline const Brush3D& GetBrush() const { return m_Brush; }

	LUX_API void DrawLine(const math::Vector3F& start, const math::Vector3F& end);
	LUX_API void DrawBox(const math::AABBoxF& box, const math::Matrix4& transform = math::Matrix4::IDENTITY);
	LUX_API void DrawMarker(const math::Vector3F& pos, float size);
	LUX_API void DrawCircle(const math::Vector3F& pos, const math::Vector3F& nor, float radius);
	LUX_API void DrawAxes(const math::Matrix4& m = math::Matrix4::IDENTITY, float scale = 1);
	LUX_API void DrawAxes(const math::Transformation& t, float scale);
	LUX_API void DrawAxes(const math::Vector3F& pos,
		const math::Vector3F& x,
		const math::Vector3F& y,
		const math::Vector3F& z,
		float scale);

	void DrawTri( const math::Triangle3F& tri)
	{
		DrawTri(tri.A, tri.B, tri.C);
	}

	LUX_API void DrawTri(
		const math::Vector3F& a,
		const math::Vector3F& b,
		const math::Vector3F& c);

	LUX_API void DrawCurve(
		const math::Sample<math::Vector3F>* points, size_t count,
		math::EEdgeHandling edgeHandling,
		u32 quality = 4);

private:
	void DrawPartialLine(
		const Pen3D& pen,
		const math::Vector3F& start, float segStart,
		const math::Vector3F& end, float segEnd);

	void DrawPartialTri(
		const Brush3D& brush,
		const math::Vector3F& a,
		const math::Vector3F& b,
		const math::Vector3F& c);

	void DrawPartialTri(
		const Brush3D& brush,
		const math::Vector3F& a,
		const math::Vector3F& b,
		const math::Vector3F& c,
		const math::Vector3F& normal);

	void FlushLineBuffer();
	void FlushTriBuffer();

private:
	math::Matrix4 m_Transform;

	float m_PolyOffset;

	Brush3D m_Brush;
	static const u32 TRI_BUFFER_SIZE = 128 * 3;
	Vertex3D m_TriBuffer[TRI_BUFFER_SIZE];
	u32 m_TriBufferCursor;

	Pen3D m_Pen;
	static const u32 LINE_BUFFER_SIZE = 128 * 2;
	Canvas3DSystem::PenVertex m_LineBuffer[LINE_BUFFER_SIZE];
	u32 m_LineBufferCursor;

	Renderer* m_Renderer;
	EPass m_LastPass;
};

} // namespace video
} // namespace lux

#endif // #ifndef INCLUDED_VIDEO_CANVAS3D_H
