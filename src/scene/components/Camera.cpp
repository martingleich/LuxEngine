#include "scene/components/Camera.h"
#include "scene/Node.h"
#include "scene/Scene.h"

#include "video/Renderer.h"

LX_REFERABLE_MEMBERS_SRC(lux::scene::Camera, "lux.comp.Camera");

namespace lux
{
namespace scene
{

void AbstractCamera::Register(bool doRegister)
{
	Component::Register(doRegister);

	if(auto s = GetScene())
		s->RegisterCamera(this, doRegister);
}

Camera::Camera() :
	m_FOV(math::AngleF::Degree(60.0f)),
	m_XMax(1.0f),
	m_Aspect(1.0f),
	m_NearPlane(0.1f),
	m_FarPlane(500.0f),
	m_IsOrtho(false),
	m_RenderPriority(0),
	m_Listener(nullptr)
{
}

Camera::~Camera()
{
}

void Camera::SetViewModification(const math::Matrix4& mod)
{
	m_ViewModification = mod;
}

const math::Matrix4& Camera::GetViewModification()
{
	return m_ViewModification;
}

void Camera::SetAspect(float aspect)
{
	m_Aspect = aspect;
}

float Camera::GetAspect() const
{
	return m_Aspect;
}

void Camera::SetFOV(math::AngleF fovY)
{
	m_FOV = fovY;
}

math::AngleF Camera::GetFOV() const
{
	return m_FOV;
}

void Camera::SetXMax(float xmax)
{
	m_XMax = xmax;
}

float Camera::GetXMax() const
{
	return m_XMax;
}

void Camera::SetNearPlane(float near)
{
	m_NearPlane = near;
}

float Camera::GetNearPlane() const
{
	return m_NearPlane;
}

void Camera::SetFarPlane(float far)
{
	m_FarPlane = far;
}

float Camera::GetFarPlane() const
{
	return m_FarPlane;
}

void Camera::SetOrtho(bool ortho)
{
	m_IsOrtho = ortho;
}

bool Camera::IsOrtho() const
{
	return m_IsOrtho;
}

void Camera::SetRenderTarget(const video::RenderTarget& target)
{
	m_RenderTarget = target;
}

const video::RenderTarget& Camera::GetRenderTarget() const
{
	return m_RenderTarget;
}

void Camera::SetRenderPriority(int p)
{
	m_RenderPriority = p;
}

int Camera::GetRenderPriority() const
{
	return m_RenderPriority;
}

void Camera::SetCameraListener(Listener* l)
{
	m_Listener = l;
}

AbstractCamera::Listener* Camera::GetCameraListener() const
{
	return m_Listener;
}

void Camera::PreRender(const SceneRenderData& r)
{
	if(m_Listener)
		m_Listener->PreRender(this);
	LUX_UNUSED(r);
}

void Camera::Render(const SceneRenderData& render)
{
	render.video->SetTransform(video::ETransform::Projection, GetProjection());
	render.video->SetTransform(video::ETransform::View, GetView());
}

void Camera::PostRender(const SceneRenderData& r)
{
	if(m_Listener)
		m_Listener->PostRender(this);
	LUX_UNUSED(r);
}

const math::ViewFrustum& Camera::GetFrustum()
{
	m_ActiveViewFrustum = CalculateViewFrustum(CalculateViewMatrix());
	return m_ActiveViewFrustum;
}

const math::Matrix4& Camera::GetView()
{
	m_ActiveView = CalculateViewMatrix();
	return m_ActiveView;
}

const math::Matrix4& Camera::GetProjection()
{
	m_ActiveProjection = CalculateProjectionMatrix();
	return m_ActiveProjection;
}

math::Matrix4 Camera::CalculateProjectionMatrix()
{
	math::Matrix4 out;
	if(m_IsOrtho) {
		out.BuildProjection_Ortho(
			m_XMax,
			m_Aspect,
			m_NearPlane,
			m_FarPlane);
	} else {
		out.BuildProjection_Persp(
			m_FOV,
			m_Aspect,
			m_NearPlane,
			m_FarPlane);
	}

	return out;
}

math::Matrix4 Camera::CalculateViewMatrix()
{
	auto n = GetNode();
	math::Vector3F position = n->GetAbsolutePosition();
	math::Vector3F direction = n->FromRelativeDir(math::Vector3F::UNIT_Z);
	math::Vector3F up = n->FromRelativeDir(math::Vector3F::UNIT_Y);

	math::Matrix4 out;
	out.BuildCameraLookAt(position, position + direction, up);
	out *= m_ViewModification;

	return out;
}

math::ViewFrustum Camera::CalculateViewFrustum(const math::Matrix4& view)
{
	math::ViewFrustum out;
	auto n = GetNode();
	if(m_IsOrtho)
		out = math::ViewFrustum::FromOrthoCam(n->GetAbsolutePosition(), view, m_XMax, m_Aspect, m_NearPlane, m_FarPlane);
	else
		out = math::ViewFrustum::FromPerspCam(n->GetAbsolutePosition(), view, m_FOV, m_Aspect, m_NearPlane, m_FarPlane);

	return out;
}

} // namespace scene
} // namespace lux
