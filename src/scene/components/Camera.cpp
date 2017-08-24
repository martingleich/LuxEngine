#include "scene/components/Camera.h"
#include "scene/Node.h"
#include "video/Renderer.h"
#include "core/ReferableRegister.h"

LUX_REGISTER_REFERABLE_CLASS("lux.comp.Camera", lux::scene::Camera);

namespace lux
{
namespace scene
{

Camera::Camera() :
	m_HasCustomProjection(false),
	m_FOV(math::AngleF::Degree(60.0f)),
	m_XMax(1.0f),
	m_Aspect(1.0f),
	m_NearPlane(0.1f),
	m_FarPlane(500.0f),
	m_IsOrtho(false),
	m_RenderPriority(0),
	m_Listener(nullptr),
	m_AutoAspect(true)
{
}

Camera::~Camera()
{
}

void Camera::SetCustomProjection(const math::Matrix4& proj, const math::ViewFrustum& frustum)
{
	m_CustomProjection = proj;
	m_CustomFrustum = frustum;
	m_HasCustomProjection = true;
}

const math::Matrix4& Camera::GetCustomProjection()
{
	return m_CustomProjection;
}

void Camera::ClearCustomProjection()
{
	m_HasCustomProjection = false;
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

void Camera::SetAutoAspect(bool automatic)
{
	m_AutoAspect = automatic;
}

bool Camera::GetAutoAspect()
{
	return m_AutoAspect;
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

void Camera::SetRenderPriority(s32 p)
{
	m_RenderPriority = p;
}

s32 Camera::GetRenderPriority() const
{
	return m_RenderPriority;
}

void Camera::SetCameraListener(Listener* l)
{
	m_Listener = l;
}

CameraBase::Listener* Camera::GetCameraListener() const
{
	return m_Listener;
}

void Camera::PreRender(video::Renderer* renderer, const Node* node)
{
	LUX_UNUSED(renderer);
	if(m_Listener)
		m_Listener->PreRender(this, node);
}

void Camera::Render(video::Renderer* r, const Node* n)
{
	m_ActiveView = CalculateViewMatrix(r, n);
	m_ActiveProjection = CalculateProjectionMatrix(r, n);
	m_ActiveViewFrustum = CalculateViewFrustum(r, n, m_ActiveView);

	r->SetTransform(video::ETransform::Projection, m_ActiveProjection);
	r->SetTransform(video::ETransform::View, m_ActiveView);
}

void Camera::PostRender(video::Renderer* renderer, const Node* node)
{
	LUX_UNUSED(renderer);
	if(m_Listener)
		m_Listener->PostRender(this, node);
}

const math::ViewFrustum& Camera::GetActiveFrustum() const
{
	return m_ActiveViewFrustum;
}

const math::Matrix4& Camera::GetActiveView() const
{
	return m_ActiveView;
}

const math::Matrix4& Camera::GetActiveProjection() const
{
	return m_ActiveProjection;
}

math::Matrix4 Camera::CalculateProjectionMatrix(video::Renderer* r, const Node* n)
{
	LUX_UNUSED(n);

	if(m_HasCustomProjection)
		return m_CustomProjection;

	float aspect;
	if(m_AutoAspect) {
		if(m_RenderTarget.GetTexture())
			aspect = m_RenderTarget.GetSize().GetAspect();
		else
			aspect = r->GetRenderTarget().GetSize().GetAspect();
	} else {
		aspect = m_Aspect;
	}

	math::Matrix4 out;
	if(m_IsOrtho) {
		out.BuildProjection_Ortho(
			m_XMax,
			aspect,
			m_NearPlane,
			m_FarPlane);
	} else {
		out.BuildProjection_Persp(
			m_FOV.Radian(),
			aspect,
			m_NearPlane,
			m_FarPlane);
	}

	return out;
}

math::Matrix4 Camera::CalculateViewMatrix(video::Renderer* r, const Node* n)
{
	LUX_UNUSED(r);
	math::Vector3F position = n->GetAbsolutePosition();
	math::Vector3F direction = n->FromRelativeDir(math::Vector3F::UNIT_Z);
	math::Vector3F up = n->FromRelativeDir(math::Vector3F::UNIT_Y);

	math::Matrix4 out;
	out.BuildCameraLookAt(position, position + direction, up);
	out *= m_ViewModification;

	return out;
}

math::ViewFrustum Camera::CalculateViewFrustum(video::Renderer* r, const Node* n, const math::Matrix4& view)
{
	LUX_UNUSED(r);

	float aspect;
	if(m_AutoAspect) {
		if(m_RenderTarget.GetTexture())
			aspect = m_RenderTarget.GetSize().GetAspect();
		else
			aspect = r->GetRenderTarget().GetSize().GetAspect();
	} else {
		aspect = m_Aspect;
	}
	math::ViewFrustum out;
	if(m_HasCustomProjection) {
		out = m_CustomFrustum;
		out.Transform(view);
	} else {
		if(m_IsOrtho)
			out = math::ViewFrustum::FromOrthoCam(n->GetAbsolutePosition(), view, m_XMax, aspect, m_NearPlane, m_FarPlane);
		else
			out = math::ViewFrustum::FromPerspCam(n->GetAbsolutePosition(), view, m_FOV, aspect, m_NearPlane, m_FarPlane);
	}

	return out;
}

core::Name Camera::GetReferableType() const
{
	return SceneComponentType::Camera;
}

StrongRef<Referable> Camera::Clone() const
{
	return LUX_NEW(Camera)(*this);
}

} // namespace scene
} // namespace lux
