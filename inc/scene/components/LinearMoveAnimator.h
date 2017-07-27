#ifndef INCLUDED_SCENE_LINEAR_MOVE_ANIMATOR_H
#define INCLUDED_SCENE_LINEAR_MOVE_ANIMATOR_H
#include "scene/components/Animator.h"
#include "math/Line3.h"

namespace lux
{
namespace scene
{

class LinearMoveAnimator : public Animator
{
public:
	LUX_API LinearMoveAnimator();
	LUX_API void SetData(
		const math::Line3F& line,
		float duration,
		bool jumpBack = false,
		u32 count = std::numeric_limits<u32>::max());

	void Animate(Node* node, float secsPassed);

	StrongRef<Referable> Clone() const;
	core::Name GetReferableType() const;

private:
	math::Line3F m_Line;
	u32 m_Count;
	float m_Period;
	float m_Time;

	bool m_Direction;
	bool m_JumpBack;
};

}
}

#endif // #ifndef INCLUDED_LINEAR_MOVE_ANIMATOR_H