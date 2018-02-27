#ifndef INCLUDED_TURNTABLE_CAMERA_CONTROL_H
#define INCLUDED_TURNTABLE_CAMERA_CONTROL_H
#include "scene/components/Animator.h"
#include "math/Vector3.h"
#include "input/InputEvent.h"

namespace lux
{
namespace scene
{

class TurntableCameraControl : public Animator
{
	LX_REFERABLE_MEMBERS_API(TurntableCameraControl, LUX_API);
public:
	LUX_API TurntableCameraControl();
	LUX_API ~TurntableCameraControl();

	LUX_API void Animate(float time);

	void RotXDelta(float x)
	{
		m_RotDelta.x += x;
	}
	void RotYDelta(float y)
	{
		m_RotDelta.y += y;
	}
	void ZoomDelta(float f)
	{
		m_ZoomDelta += f;
	}

	void RotXRate(float x)
	{
		m_RotRate.x = x;
	}
	void RotYRate(float y)
	{
		m_RotRate.y = y;
	}
	void ZoomRate(float f)
	{
		m_ZoomRate = f;
	}

	LUX_API void HandleInput(const input::Event& event);

	LUX_API void EnableInput();
	LUX_API void DisableInput();
	
	bool IsInputActive()
	{
		return m_IsControlActive;
	}

	void SetOrbitCenter(const math::Vector3F& center)
	{
		m_OrbitCenter = center;
	}
	const math::Vector3F& GetOrbitCenter() const
	{
		return m_OrbitCenter;
	}

	void SetTableNormal(const math::Vector3F& normal)
	{
		m_TableNormal = normal.Normal();
	}
	const math::Vector3F& GetTableNormal() const
	{
		return m_TableNormal;
	}

private:
	math::Vector2F m_RotDelta;
	float m_ZoomDelta;

	math::Vector2F m_RotRate;
	float m_ZoomRate;

	float m_ZoomKeySpeed;
	float m_RotKeySpeed;

	float m_ZoomSpeed;
	float m_RotSpeed;

	math::Vector3F m_OrbitCenter;
	math::Vector3F m_TableNormal;

	math::Vector3F m_InputVector;
	bool m_IsControlActive;
};

} // namespace scene
} // namespace lux

#endif // #ifndef INCLUDED_TURNTABLE_CAMERA_CONTROL_H