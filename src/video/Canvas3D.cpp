#include "video/Canvas3D.h"
#include "video/Renderer.h"
#include "video/VideoDriver.h"
#include "video/MaterialLibrary.h"

namespace lux
{
namespace video
{

static StrongRef<Canvas3DSystem> g_Canvas3DSystem;

Canvas3DSystem::Canvas3DSystem()
{
	m_PenPass.lighting = video::ELightingFlag::Disabled;
	m_PenPass.fogEnabled = false;
	m_PenPass.shader = video::MaterialLibrary::Instance()->GetFixedFunctionShader({}, {}, true);

	video::VertexDeclaration decl;
	decl.AddElement(video::VertexElement::EUsage::Position, video::VertexElement::EType::Float3);
	decl.AddElement(video::VertexElement::EUsage::Diffuse, video::VertexElement::EType::Color);
	m_PenFormat = video::VertexFormat("penVertex", decl);

	m_BrushPass.lighting = video::ELightingFlag::Disabled;
	m_BrushPass.fogEnabled = false;
	m_BrushPass.culling = video::EFaceSide::None;
	m_BrushPass.zWriteEnabled = false;
	m_BrushPass.alpha.srcFactor = video::EBlendFactor::SrcAlpha;
	m_BrushPass.alpha.dstFactor = video::EBlendFactor::OneMinusSrcAlpha;
	m_BrushPass.alpha.blendOperator = video::EBlendOperator::Add;
	m_BrushPass.shader = video::MaterialLibrary::Instance()->GetFixedFunctionShader({}, {}, true);
}

Canvas3DSystem::~Canvas3DSystem()
{
}

void Canvas3DSystem::Initialize()
{
	g_Canvas3DSystem = LUX_NEW(Canvas3DSystem);
}

void Canvas3DSystem::Destroy()
{
	g_Canvas3DSystem = nullptr;
}

Canvas3DSystem* Canvas3DSystem::Instance()
{
	return g_Canvas3DSystem;
}

const Pass& Canvas3DSystem::GetPenPass() const
{
	return m_PenPass;
}

const Pass& Canvas3DSystem::GetBrushPass() const
{
	return m_BrushPass;
}

const VertexFormat& Canvas3DSystem::GetPenVertexFormat() const
{
	return m_PenFormat;
}

const VertexFormat& Canvas3DSystem::GetBrushVertexFormat() const
{
	return VertexFormat::STANDARD;
}

///////////////////////////////////////////////////////////////////////////////

Canvas3D::Canvas3D(const math::Matrix4& transform, float polyOffset, video::Renderer* renderer) :
	m_Transform(transform),
	m_PolyOffset(polyOffset),
	m_TriBufferCursor(0),
	m_LineBufferCursor(0),
	m_Renderer(renderer ? renderer : (video::Renderer*)video::VideoDriver::Instance()->GetRenderer()),
	m_LastPass(NonePass)
{
}

Canvas3D::~Canvas3D()
{
	FlushLineBuffer();
	FlushTriBuffer();
}

void Canvas3D::DrawLine(const math::Vector3F& start, const math::Vector3F& end)
{
	DrawPartialLine(m_Pen, start, 0, end, start.GetDistanceTo(end));
}

void Canvas3D::DrawBox(const math::AABBoxF& box, const math::Matrix4& transform)
{
	math::Vector3F corners[8];
	box.GetCorners(corners);
	if(&transform != &math::Matrix4::IDENTITY)
		transform.TransformVectorArray(corners, corners, 8);
	/*
	   3------7
	  /|     /|
	 / |    / |
	2--+---6  |
	|  1---+--5
	| /    | /
	|/     |/
	0------4
	*/
	if(m_Pen.GetColor().GetAlpha()) {
		DrawLine(corners[0], corners[1]);
		DrawLine(corners[1], corners[3]);
		DrawLine(corners[3], corners[2]);
		DrawLine(corners[2], corners[0]);

		DrawLine(corners[4], corners[5]);
		DrawLine(corners[5], corners[7]);
		DrawLine(corners[7], corners[6]);
		DrawLine(corners[6], corners[4]);

		DrawLine(corners[0], corners[4]);
		DrawLine(corners[1], corners[5]);
		DrawLine(corners[3], corners[7]);
		DrawLine(corners[2], corners[6]);
	}
	if(m_Brush.GetColor().GetAlpha()) {
		auto norX = (corners[4] - corners[0]).Normal();
		auto norY = (corners[2] - corners[0]).Normal();
		auto norZ = (corners[1] - corners[0]).Normal();
		DrawPartialTri(m_Brush, corners[0], corners[1], corners[2], -norX);
		DrawPartialTri(m_Brush, corners[1], corners[2], corners[3], -norX);
		DrawPartialTri(m_Brush, corners[4], corners[5], corners[6], norX);
		DrawPartialTri(m_Brush, corners[5], corners[6], corners[7], norX);

		DrawPartialTri(m_Brush, corners[0], corners[4], corners[2], -norZ);
		DrawPartialTri(m_Brush, corners[4], corners[2], corners[6], -norZ);
		DrawPartialTri(m_Brush, corners[1], corners[5], corners[3], norZ);
		DrawPartialTri(m_Brush, corners[5], corners[3], corners[7], -norZ);

		DrawPartialTri(m_Brush, corners[0], corners[1], corners[4], -norY);
		DrawPartialTri(m_Brush, corners[1], corners[4], corners[5], -norY);
		DrawPartialTri(m_Brush, corners[2], corners[6], corners[3], norY);
		DrawPartialTri(m_Brush, corners[3], corners[6], corners[7], norY);
	}
}

void Canvas3D::DrawMarker(const math::Vector3F& pos, float size)
{
	float ext = 0.7f;
	math::Vector3F up = math::Vector3F::UNIT_Y*size;
	math::Vector3F sidex = math::Vector3F::UNIT_X*size*ext;
	math::Vector3F sidez = math::Vector3F::UNIT_Z*size*ext;
	if(m_Pen.GetColor().GetAlpha()) {
		DrawLine(pos + up, pos + sidex);
		DrawLine(pos + up, pos + sidez);
		DrawLine(pos + up, pos - sidex);
		DrawLine(pos + up, pos - sidez);

		DrawLine(pos - up, pos + sidex);
		DrawLine(pos - up, pos + sidez);
		DrawLine(pos - up, pos - sidex);
		DrawLine(pos - up, pos - sidez);

		DrawLine(pos + sidex, pos + sidez);
		DrawLine(pos + sidex, pos - sidez);
		DrawLine(pos - sidex, pos + sidez);
		DrawLine(pos - sidex, pos - sidez);
	}

	if(m_Brush.GetColor().GetAlpha()) {
		DrawPartialTri(m_Brush, pos + up, pos + sidez, pos + sidex);
		DrawPartialTri(m_Brush, pos + up, pos + sidez, pos - sidex);
		DrawPartialTri(m_Brush, pos + up, pos - sidez, pos + sidex);
		DrawPartialTri(m_Brush, pos + up, pos - sidez, pos - sidex);

		DrawPartialTri(m_Brush, pos - up, pos + sidez, pos + sidex);
		DrawPartialTri(m_Brush, pos - up, pos + sidez, pos - sidex);
		DrawPartialTri(m_Brush, pos - up, pos - sidez, pos + sidex);
		DrawPartialTri(m_Brush, pos - up, pos - sidez, pos - sidex);
	}
}

void Canvas3D::DrawCircle(const math::Vector3F& pos, const math::Vector3F& _nor, float radius)
{
	int res = 32;
	auto nor = _nor.Normal();
	math::Vector3F ux = nor.GetOrthoNormal();
	math::Vector3F uy = nor.Cross(ux);
	math::Vector3F start = pos + radius*ux;
	math::Vector3F last = start;
	float lastSeg = 0;
	for(int i = 1; i < res; ++i) {
		float t = (i*math::Constants<float>::two_pi()) / res;
		math::Vector3F v = pos + radius*std::cos(t)*ux + radius*std::sin(t)*uy;
		float seg = lastSeg + last.GetDistanceTo(v);
		DrawPartialLine(m_Pen, last, lastSeg,
			v, seg);
		last = v;
		lastSeg = seg;
	}
	float seg = lastSeg + last.GetDistanceTo(start);
	DrawPartialLine(m_Pen, last, lastSeg, start, seg);
}

void Canvas3D::DrawAxes(const math::Matrix4& m, float scale)
{
	DrawAxes(
		m.GetTranslation(),
		m.GetAxisX(),
		m.GetAxisY(),
		m.GetAxisZ(),
		scale);
}

void Canvas3D::DrawAxes(const math::Transformation& t, float scale)
{
	DrawAxes(
		t.translation,
		t.TransformDir(math::Vector3F::UNIT_X),
		t.TransformDir(math::Vector3F::UNIT_Y),
		t.TransformDir(math::Vector3F::UNIT_Z),
		scale);
}

void Canvas3D::DrawAxes(const math::Vector3F& pos,
	const math::Vector3F& x,
	const math::Vector3F& y,
	const math::Vector3F& z,
	float scale)
{
	Pen3D redPen(video::Color::Red);
	Pen3D bluePen(video::Color::Blue);
	Pen3D greenPen(video::Color::Green);
	DrawPartialLine(redPen, pos, 0, pos + scale*x, scale*x.GetLength());
	DrawPartialLine(bluePen, pos, 0, pos + scale*z, scale*z.GetLength());
	DrawPartialLine(greenPen, pos, 0, pos + scale*y, scale*y.GetLength());
}

void Canvas3D::DrawTri(
	const math::Vector3F& a,
	const math::Vector3F& b,
	const math::Vector3F& c)
{
	if(m_Brush.GetColor().GetAlpha())
		DrawPartialTri(m_Brush, a, b, c);

	if(m_Pen.GetColor().GetAlpha()) {
		DrawLine(a, b);
		DrawLine(a, c);
		DrawLine(b, c);
	}
}

void Canvas3D::DrawCurve(
	const math::Sample<math::Vector3F>* points, int count,
	math::EEdgeHandling edgeHandling,
	int quality)
{
	int samples = count * quality;
	auto getSample = [&](float x) {
		return math::CurveInterpolation(
			points, count, x, edgeHandling,
			math::EInterpolation::Smooth);
	};

	math::Vector3F start = getSample(0);
	math::Vector3F last = start;
	float lastSeg = 0;
	for(int i = 1; i < samples; ++i) {
		math::Vector3F v = getSample((float)i / (samples - 1));
		float seg = lastSeg + last.GetDistanceTo(v);
		DrawPartialLine(m_Pen, last, lastSeg,
			v, seg);
		last = v;
		lastSeg = seg;
	}
}

void Canvas3D::DrawPartialLine(
	const Pen3D& pen,
	const math::Vector3F& start, float segStart,
	const math::Vector3F& end, float segEnd)
{
	LUX_UNUSED(segStart);
	LUX_UNUSED(segEnd);

	if(m_LineBufferCursor == LINE_BUFFER_SIZE)
		FlushLineBuffer();

	m_LineBuffer[m_LineBufferCursor++] = Canvas3DSystem::PenVertex(start, pen.GetColor());
	m_LineBuffer[m_LineBufferCursor++] = Canvas3DSystem::PenVertex(end, pen.GetColor());
}

void Canvas3D::DrawPartialTri(
	const Brush3D& brush,
	const math::Vector3F& a,
	const math::Vector3F& b,
	const math::Vector3F& c)
{
	DrawPartialTri(brush, a, b, c, ((b - a).Cross(c - a)).Normal());
}

void Canvas3D::DrawPartialTri(
	const Brush3D& brush,
	const math::Vector3F& a,
	const math::Vector3F& b,
	const math::Vector3F& c,
	const math::Vector3F& normal)
{
	if(m_TriBufferCursor == TRI_BUFFER_SIZE)
		FlushTriBuffer();

	m_TriBuffer[m_TriBufferCursor++] = video::Vertex3D(a, brush.GetColor(), normal, math::Vector2F(0, 0));
	m_TriBuffer[m_TriBufferCursor++] = video::Vertex3D(b, brush.GetColor(), normal, math::Vector2F(0, 0));
	m_TriBuffer[m_TriBufferCursor++] = video::Vertex3D(c, brush.GetColor(), normal, math::Vector2F(0, 0));
}

void Canvas3D::FlushLineBuffer()
{
	if(m_LineBufferCursor == 0)
		return;
	lxAssert(m_LineBufferCursor % 2 == 0);

	if(m_LastPass != PenPass) {
		m_Renderer->SetPass(Canvas3DSystem::Instance()->GetPenPass(), false);
		m_Renderer->SetTransform(video::ETransform::World, m_Transform);
	}

	m_Renderer->Draw3DPrimitiveList(
		EPrimitiveType::Lines,
		m_LineBufferCursor / 2,
		m_LineBuffer,
		m_LineBufferCursor,
		Canvas3DSystem::Instance()->GetPenVertexFormat());
	m_LineBufferCursor = 0;
	m_LastPass = PenPass;
}

void Canvas3D::FlushTriBuffer()
{
	if(m_TriBufferCursor == 0)
		return;
	lxAssert(m_TriBufferCursor % 3 == 0);

	if(m_LastPass != BrushPass) {
		auto pass = Canvas3DSystem::Instance()->GetBrushPass();
		pass.polygonOffset = m_PolyOffset;
		m_Renderer->SetPass(pass, false, nullptr);
		m_Renderer->SetTransform(video::ETransform::World, m_Transform);
	}

	m_Renderer->Draw3DPrimitiveList(
		EPrimitiveType::Triangles,
		m_TriBufferCursor / 3,
		m_TriBuffer,
		m_TriBufferCursor,
		Canvas3DSystem::Instance()->GetBrushVertexFormat());
	m_TriBufferCursor = 0;
	m_LastPass = BrushPass;
}

} // namespace video
} // namespace lux
