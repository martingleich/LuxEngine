#include "LinearMoveAnimator.h"
#include "scene/SceneNode.h"

namespace lux
{
namespace scene
{

LinearMoveComponent::LinearMoveComponent(
	const math::line3df& line,
	float duration,
	bool jumpBack,
	u32 count) :
	m_Line(line),
	m_Period(2 * duration),
	m_Time(0.0f),
	m_Count(count),
	m_Direction(false),
	m_JumpBack(jumpBack)
{
	if(m_Count != std::numeric_limits<u32>::max())
		m_Count++;
}

void LinearMoveComponent::Animate(float secsPassed)
{
	if(m_Count == 0) {
		GetParent()->MarkForDelete(this);
		return;
	}

	auto node = GetParent();

	m_Time = fmodf(m_Time + secsPassed, m_Period);

	float t;
	bool dir = (m_Time <= m_Period / 2);
	if(dir)
		t = m_Time / (m_Period / 2);
	else {
		if(m_JumpBack)
			t = m_Time / (m_Period / 2) - 1;
		else
			t = 2 - m_Time / (m_Period / 2);
	}
	if(dir != m_Direction) {
		if(m_Count != std::numeric_limits<u32>::max())
			--m_Count;
		m_Direction = dir;
		if(m_Count == 0) {
			GetParent()->MarkForDelete(this);
			return;
		}
	}

	node->SetPosition(math::Lerp(m_Line.start, m_Line.end, t));
}

}
}
