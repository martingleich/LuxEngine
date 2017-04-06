#ifndef INCLUDED_CCAMERASCENENODE_H
#define INCLUDED_CCAMERASCENENODE_H

#include "scene/nodes/CameraSceneNode.h"

namespace lux
{
namespace scene
{

class CameraSceneNodeImpl : public CameraSceneNode
{
public:
	CameraSceneNodeImpl();
	CameraSceneNodeImpl(const CameraSceneNodeImpl& other) = default;

	// Setzt die Projektionsmatrix neu
	virtual void SetProjection(const math::matrix4& projection);
	// Fragt die Projektionsmatrix ab
	virtual const math::matrix4& GetProjection() const;

	// Fragt die Sichtmatrix ab
	virtual const math::matrix4& GetView() const;

	// Setzt einen Kameramodifizierer(damit kann die Sichtmatrix nach allen Berechnungen noch verändert werden
	// für z.B. Spiegelungen)
	virtual void SetViewModificator(const math::matrix4 modification);
	// Fragt den Kameramodifikator ab(damit kann die Sichtmatrix nach allen Berechnungen noch verändert werden
	// für z.B. Spiegelungen)
	virtual const math::matrix4& GetViewModificator() const;

	// Setzt das Bildseitenverhältnis(Breite/Höhe) neu
	virtual void SetAspect(float aspect);
	// Fragt das Bildseitenverhältnis(Breite/Höhe) ab
	virtual float GetAspect() const;

	// Setzt den vertikalen Sichtwinkel in rad neu
	virtual void SetFOV(float fieldOfVison);
	// Fragt den vertikalen Sichtwinkel in rad ab
	virtual float GetFOV() const;

	// Setzt die nahe Clippingebene neu
	virtual void SetNearPlane(float near);
	// Fragt die nahe Clippingebene ab
	virtual float GetNearPlane() const;

	// Setzt die ferne Clippingebene neu
	virtual void SetFarPlane(float far);
	// Fragt die ferne Clippingebene ab
	virtual float GetFarPlane() const;

	// Setzt den Up-Vektor neu
	virtual void SetUpVector(const math::vector3f& upVector);
	// Fragt den Up-Vektor ab
	virtual const math::vector3f& GetUpVector() const;
	bool SetSceneManager(SceneManager* mngr);
	virtual void OnRegisterSceneNode();
	virtual void Render();

	core::Name GetReferableSubType() const;
	StrongRef<Referable> Clone() const;

private:
	void RecalculateProjectionMatrix();
	void RecalculateViewMatrix();

	math::vector3f m_vUpVector;

	math::matrix4 m_mProjectionMatrix;
	math::matrix4 m_mViewMatrix;
	math::matrix4 m_mViewModificator;

	float m_fFOV;
	float m_fAspect;
	float m_fNearPlane;
	float m_fFarPlane;
};

}    // namespace scene
}    // namespace lux

#endif