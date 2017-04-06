#ifndef INCLUDED_ICAMERASCENENODE_H
#define INCLUDED_ICAMERASCENENODE_H

#include "scene/SceneNode.h"

namespace lux
{
namespace scene
{

//! Represent a camera in the scenegraph
class CameraSceneNode : public SceneNode
{
public:
	// Setzt die Projektionsmatrix neu
	virtual void SetProjection(const math::matrix4& projection) = 0;
	// Fragt die Projektionsmatrix ab
	virtual const math::matrix4& GetProjection() const = 0;

	// Fragt die Sichtmatrix ab
	virtual const math::matrix4& GetView() const = 0;

	// Setzt einen Kameramodifizierer(damit kann die Sichtmatrix nach allen Berechnungen noch verändert werden
	// für z.B. Spiegelungen)
	virtual void SetViewModificator(const math::matrix4 modification) = 0;
	// Fragt den Kameramodifikator ab(damit kann die Sichtmatrix nach allen Berechnungen noch verändert werden
	// für z.B. Spiegelungen)
	virtual const math::matrix4& GetViewModificator() const = 0;

	// Setzt das Bildseitenverhältnis(Breite/Höhe) neu
	virtual void SetAspect(float aspect) = 0;
	// Fragt das Bildseitenverhältnis(Breite/Höhe) ab
	virtual float GetAspect() const = 0;

	// Setzt den vertikalen Sichtwinkel in rad neu
	virtual void SetFOV(float fieldOfVison) = 0;
	// Fragt den vertikalen Sichtwinkel in rad ab
	virtual float GetFOV() const = 0;

	// Setzt die nahe Clippingebene neu
	virtual void SetNearPlane(float near) = 0;
	// Fragt die nahe Clippingebene ab
	virtual float GetNearPlane() const = 0;

	// Setzt die ferne Clippingebene neu
	virtual void SetFarPlane(float far) = 0;
	// Fragt die ferne Clippingebene ab
	virtual float GetFarPlane() const = 0;

	// Setzt den Up-Vektor neu
	virtual void SetUpVector(const math::vector3f& upVector) = 0;
	// Fragt den Up-Vektor ab
	virtual const math::vector3f& GetUpVector() const = 0;

	virtual StrongRef<Referable> Clone() const = 0;
};

}    // namespace scene
}    // namespace lux

#endif