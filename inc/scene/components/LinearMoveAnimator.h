#ifndef INCLUDED_LUX_SCENE_LINEAR_MOVE_ANIMATOR_H
#define INCLUDED_LUX_SCENE_LINEAR_MOVE_ANIMATOR_H
#include "scene/components/Animator.h"
#include "math/Line3.h"

namespace lux
{
namespace scene
{

class LinearMoveAnimator : public Animator
{
	LX_REFERABLE_MEMBERS_API(LinearMoveAnimator, LUX_API);
public:
	LUX_API LinearMoveAnimator();
	LUX_API void SetData(
		const math::Line3F& line,
		float duration,
		bool jumpBack = false,
		int count = -1);

	void Animate(float secsPassed);

private:
	math::Line3F m_Line;
	int m_Count;
	float m_Period;
	float m_Time;

	bool m_Direction;
	bool m_JumpBack;
};

}
}

#endif // #ifndef INCLUDED_LUX_LINEAR_MOVE_ANIMATOR_H