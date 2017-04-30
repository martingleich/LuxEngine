#include "CameraSceneNodeImpl.h"
#include "scene/SceneManager.h"
#include "video/VideoDriver.h"
#include "core/Logger.h"
#include "core/ReferableRegister.h"

LUX_REGISTER_REFERABLE_CLASS(lux::scene::CameraSceneNodeImpl)

namespace lux
{
namespace scene
{

CameraSceneNodeImpl::CameraSceneNodeImpl() :
	m_HasCustomProjection(false),
	m_FOV(math::DegToRad(60.0f)),
	m_XMax(1.0f),
	m_Aspect(1.0f),
	m_NearPlane(0.1f),
	m_FarPlane(500.0f),
	m_IsOrtho(false),
	m_RenderPriority(0),
	m_BackgroundColor(video::Color::Black),
	m_AutoAspect(true)
{
}

void CameraSceneNodeImpl::SetCustomProjection(const math::matrix4& proj)
{
	m_Projection = proj;
	m_HasCustomProjection = true;
}

void CameraSceneNodeImpl::ClearCustomProjection()
{
	m_HasCustomProjection = false;
	RecalculateProjectionMatrix();
}

const math::matrix4& CameraSceneNodeImpl::GetProjection() const
{
	return m_Projection;
}

const math::matrix4& CameraSceneNodeImpl::GetView() const
{
	return m_View;
}

void CameraSceneNodeImpl::SetViewModification(const math::matrix4& mod)
{
	m_ViewModification = mod;
}

const math::matrix4& CameraSceneNodeImpl::GetViewModification()
{
	return m_ViewModification;
}

void CameraSceneNodeImpl::SetAspect(float aspect)
{
	m_Aspect = aspect;
	RecalculateProjectionMatrix();
}

float CameraSceneNodeImpl::GetAspect() const
{
	return m_Aspect;
}

void CameraSceneNodeImpl::SetFOV(float fov)
{
	m_FOV = fov;
	RecalculateProjectionMatrix();
}

float CameraSceneNodeImpl::GetFOV() const
{
	return m_FOV;
}

void CameraSceneNodeImpl::SetXMax(float xmax)
{
	m_XMax = xmax;
	RecalculateProjectionMatrix();
}

float CameraSceneNodeImpl::GetXMax() const
{
	return m_XMax;
}

void CameraSceneNodeImpl::SetNearPlane(float near)
{
	m_NearPlane = near;
	RecalculateProjectionMatrix();
}

float CameraSceneNodeImpl::GetNearPlane() const
{
	return m_NearPlane;
}

void CameraSceneNodeImpl::SetFarPlane(float far)
{
	m_FarPlane = far;
	RecalculateProjectionMatrix();
}

float CameraSceneNodeImpl::GetFarPlane() const
{
	return m_FarPlane;
}

void CameraSceneNodeImpl::SetOrtho(bool ortho)
{
	m_IsOrtho = ortho;
	RecalculateProjectionMatrix();
}

void CameraSceneNodeImpl::SetAutoAspect(bool automatic)
{
	m_AutoAspect = automatic;
}

bool CameraSceneNodeImpl::GetAutoAspect()
{
	return m_AutoAspect;
}

bool CameraSceneNodeImpl::IsOrtho() const
{
	return m_IsOrtho;
}

void CameraSceneNodeImpl::SetVirtualRoot(SceneNode* node)
{
	m_VirtualRoot = node;
}

SceneNode* CameraSceneNodeImpl::GetVirtualRoot() const
{
	return m_VirtualRoot;
}

void CameraSceneNodeImpl::SetRenderTarget(const video::RenderTarget& target)
{
	m_RenderTarget = target;
	RecalculateProjectionMatrix();
}

const video::RenderTarget& CameraSceneNodeImpl::GetRenderTarget() const
{
	return m_RenderTarget;
}

void CameraSceneNodeImpl::SetRenderPriority(s32 p)
{
	m_RenderPriority = p;
}

s32 CameraSceneNodeImpl::GetRenderPriority() const
{
	return m_RenderPriority;
}

void CameraSceneNodeImpl::SetBackgroundColor(video::Color color)
{
	m_BackgroundColor = color;
}

video::Color CameraSceneNodeImpl::GetBackgroundColor()
{
	return m_BackgroundColor;
}

void CameraSceneNodeImpl::Render()
{
	RecalculateViewMatrix();
	RecalculateProjectionMatrix();

	video::VideoDriver* driver = GetSceneManager()->GetDriver();
	driver->SetTransform(video::ETS_PROJECTION, m_Projection);
	driver->SetTransform(video::ETS_VIEW, m_View);
}

void CameraSceneNodeImpl::RecalculateProjectionMatrix()
{
	if(m_AutoAspect) {
		if(m_RenderTarget)
			m_Aspect = m_RenderTarget.GetSize().GetAspect();
		else
			m_Aspect = GetSceneManager()->GetDriver()->GetRenderTarget().GetSize().GetAspect();
	}

	if(m_HasCustomProjection)
		return;

	if(m_IsOrtho) {
		m_Projection.BuildProjection_Ortho(
			m_XMax,
			m_Aspect,
			m_NearPlane,
			m_FarPlane);
	} else {
		m_Projection.BuildProjection_Persp(
			m_FOV,
			m_Aspect,
			m_NearPlane,
			m_FarPlane);
	}
}

void CameraSceneNodeImpl::RecalculateViewMatrix()
{
	math::vector3f position = GetAbsolutePosition();
	math::vector3f direction = this->FromRelativeDir(math::vector3f::UNIT_Z);
	math::vector3f up = this->FromRelativeDir(math::vector3f::UNIT_Y);

	m_View.BuildCameraLookAt(position, position + direction, up);

	m_View *= m_ViewModification;
}

bool CameraSceneNodeImpl::SetSceneManager(SceneManager* smgr)
{
	auto oldSmgr = GetSceneManager();
	if(oldSmgr)
		oldSmgr->UnRegisterCamera(this);
	if(smgr)
		smgr->RegisterCamera(this);

	return SceneNode::SetSceneManager(smgr);
}

core::Name CameraSceneNodeImpl::GetReferableSubType() const
{
	return SceneNodeType::Camera;
}

StrongRef<Referable> CameraSceneNodeImpl::Clone() const
{
	return LUX_NEW(CameraSceneNodeImpl)(*this);
}

}

}

