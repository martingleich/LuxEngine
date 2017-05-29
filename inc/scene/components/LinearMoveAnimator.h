#ifndef INCLUDED_LINEAR_MOVE_ANIMATOR_H
#define INCLUDED_LINEAR_MOVE_ANIMATOR_H
#include "scene/SceneNodeComponent.h"
#include "math/line3d.h"

namespace lux
{
namespace scene
{

class LinearMoveAnimator : public scene::AnimatedSceneNodeComponent
{
public:
	LUX_API LinearMoveAnimator();
	LUX_API void SetData(
		const math::line3df& line,
		float duration,
		bool jumpBack = false,
		u32 count = std::numeric_limits<u32>::max());

	void Animate(float secsPassed);

	StrongRef<Referable> Clone() const;
	core::Name GetReferableSubType() const;

private:
	math::line3df m_Line;
	u32 m_Count;
	float m_Period;
	float m_Time;

	bool m_Direction;
	bool m_JumpBack;
};

}
}

#endif // #ifndef INCLUDED_LINEAR_MOVE_ANIMATOR_H