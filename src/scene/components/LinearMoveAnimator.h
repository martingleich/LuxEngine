#ifndef INCLUDED_LINEAR_MOVE_ANIMATOR_H
#define INCLUDED_LINEAR_MOVE_ANIMATOR_H
#include "scene/SceneNodeComponent.h"
#include "math/line3d.h"

namespace lux
{
namespace scene
{

class LinearMoveComponent : public scene::AnimatedSceneNodeComponent
{
public:
	LinearMoveComponent() :
		LinearMoveComponent(math::line3df(), 1.0f)
	{
	}

	LinearMoveComponent(
		const math::line3df& line,
		float duration,
		bool jumpBack = false,
		u32 count = std::numeric_limits<u32>::max());

	void Init(
		const math::line3df& line,
		float duration,
		bool jumpBack,
		u32 count);

	void Animate(float secsPassed);

	StrongRef<Referable> Clone() const
	{
		return LUX_NEW(LinearMoveComponent)(*this);
	}

	core::Name GetReferableSubType() const
	{
		return SceneNodeComponentType::LinearMove;
	}

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