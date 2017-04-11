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

//****************************************************************
// Konstruktor
CameraSceneNodeImpl::CameraSceneNodeImpl() :
	m_vUpVector(math::vector3f::UNIT_Y),
	m_fNearPlane(1.0f),
	m_fFarPlane(500.0f),
	m_fFOV(math::DegToRad(60.0f)),
	m_mViewModificator(math::matrix4::IDENTITY),
	m_fAspect(1.0f)
{
	SetPosition(math::vector3f::ZERO);
	SetDirection(math::vector3f::UNIT_Z);

	RecalculateProjectionMatrix();
	RecalculateViewMatrix();
}

bool CameraSceneNodeImpl::SetSceneManager(SceneManager* mngr)
{
	SetAspect(mngr->GetDriver()->GetScreenSize().GetAspect());
	return CameraSceneNode::SetSceneManager(mngr);
}

// Setzt die Projektionsmatrix neu
void CameraSceneNodeImpl::SetProjection(const math::matrix4& projection)
{
	m_mProjectionMatrix = projection;
}

// Fragt die Projektionsmatrix ab
const math::matrix4& CameraSceneNodeImpl::GetProjection() const
{
	return m_mProjectionMatrix;
}

// Fragt die Sichtmatrix ab
const math::matrix4& CameraSceneNodeImpl::GetView() const
{
	return m_mViewMatrix;
}

// Setzt einen Kameramodifizierer(damit kann die Sichtmatrix nach allen Berechnungen noch verändert werden
// für z.B. Spiegelungen)
void CameraSceneNodeImpl::SetViewModificator(const math::matrix4 modification)
{
	m_mViewModificator = modification;
}

// Fragt den Kameramodifikator ab(damit kann die Sichtmatrix nach allen Berechnungen noch verändert werden
// für z.B. Spiegelungen)
const math::matrix4& CameraSceneNodeImpl::GetViewModificator() const
{
	return m_mViewModificator;
}

// Setzt das Bildseitenverhältnis(Breite/Höhe) neu
void CameraSceneNodeImpl::SetAspect(float aspect)
{
	m_fAspect = aspect;
	RecalculateProjectionMatrix();
}

// Fragt das Bildseitenverhältnis(Breite/Höhe) ab
float CameraSceneNodeImpl::GetAspect() const
{
	return m_fAspect;
}

// Setzt den vertikalen Sichtwinkel in rad neu
void CameraSceneNodeImpl::SetFOV(float fieldOfVison)
{
	m_fFOV = fieldOfVison;
	RecalculateProjectionMatrix();
}

// Fragt den vertikalen Sichtwinkel in rad ab
float CameraSceneNodeImpl::GetFOV() const
{
	return m_fFOV;
}

// Setzt die nahe Clippingebene neu
void CameraSceneNodeImpl::SetNearPlane(float near)
{
	m_fNearPlane = near;
	RecalculateProjectionMatrix();
}

// Fragt die nahe Clippingebene ab
float CameraSceneNodeImpl::GetNearPlane() const
{
	return m_fNearPlane;
}

// Setzt die ferne Clippingebene neu
void CameraSceneNodeImpl::SetFarPlane(float far)
{
	m_fFarPlane = far;
	RecalculateProjectionMatrix();
}

// Fragt die ferne Clippingebene ab
float CameraSceneNodeImpl::GetFarPlane() const
{
	return m_fFarPlane;
}

// Setzt den Up-Vektor neu
void CameraSceneNodeImpl::SetUpVector(const math::vector3f& upVector)
{
	m_vUpVector = upVector;
}

// Fragt den Up-Vektor ab
const math::vector3f& CameraSceneNodeImpl::GetUpVector() const
{
	return m_vUpVector;
}

// Registiert den Scenenode zum Zeichnen
void CameraSceneNodeImpl::OnRegisterSceneNode()
{
	// Nur die aktive Kamera zeichnen
	if(GetSceneManager()->GetActiveCamera() == this)
		GetSceneManager()->RegisterNodeForRendering(this, ESNRP_CAMERA);

	// An Kinder weitergeben
	SceneNode::OnRegisterSceneNode();
}

// Zeichnet die Kamera
void CameraSceneNodeImpl::Render()
{
	// Die Sichtmatrix neu berechnen
	RecalculateViewMatrix();

	video::VideoDriver* driver = GetSceneManager()->GetDriver();
	driver->SetTransform(video::ETS_PROJECTION, m_mProjectionMatrix);
	driver->SetTransform(video::ETS_VIEW, m_mViewMatrix);
}

// Berechnet die Projektionsmatrix neu
void CameraSceneNodeImpl::RecalculateProjectionMatrix()
{
	m_mProjectionMatrix.BuildProjection_Persp(m_fFOV, m_fAspect, m_fNearPlane, m_fFarPlane);
}

// Berechnet die Kameramatrix neu
void CameraSceneNodeImpl::RecalculateViewMatrix()
{
	math::vector3f Position = GetAbsolutePosition();
	math::vector3f Direction = this->FromRelativeDir(math::vector3f::UNIT_Z);

	// Wenn der Nach-oben-Vektor und der Blickpunkt das Gleiche sind gibt es ein Problem
	math::vector3f upVector = m_vUpVector;
	upVector.Normalize();

	float fDot = Direction.Dot(upVector);

	if(math::IsEqual(fabsf(fDot), 1.0f)) {
		// Die Richtung ist egal
		upVector.x += 0.5f;
	}

	// Kameramatrix bilden
	m_mViewMatrix.BuildCameraLookAt(Position, Position + Direction, m_vUpVector);

	// Affektor hinzufügen
	m_mViewMatrix *= m_mViewModificator;
}

core::Name CameraSceneNodeImpl::GetReferableSubType() const
{
	return SceneNodeType::Camera;
}

StrongRef<Referable> CameraSceneNodeImpl::Clone() const
{
	StrongRef<CameraSceneNodeImpl> out = LUX_NEW(CameraSceneNodeImpl)(*this);

	out->RecalculateProjectionMatrix();
	out->RecalculateViewMatrix();

	return out;
}

}

}

