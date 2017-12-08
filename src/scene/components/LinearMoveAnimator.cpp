#include "scene/components/LinearMoveAnimator.h"
#include "scene/Node.h"

LX_REFERABLE_MEMBERS_SRC(lux::scene::LinearMoveAnimator, lux::scene::SceneComponentType::LinearMove);

namespace lux
{
namespace scene
{

LinearMoveAnimator::LinearMoveAnimator()
{
	SetData(math::Line3F(), 1.0f);
}

void LinearMoveAnimator::SetData(
	const math::Line3F& line,
	float duration,
	bool jumpBack,
	u32 count)
{
	m_Line = line;
	m_Period = 2 * duration;
	m_JumpBack = jumpBack;
	m_Direction = false;
	m_Time = 0.0f;
	m_Count = count;

	if(m_Count != std::numeric_limits<u32>::max())
		m_Count++;
}

void LinearMoveAnimator::Animate(float secsPassed)
{
	auto node = GetParent();
	if(!node)
		return;
	if(m_Count == 0) {
		node->MarkForDelete(this);
		return;
	}

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
			node->MarkForDelete(this);
			return;
		}
	}

	node->SetPosition(math::Lerp(m_Line.start, m_Line.end, t));
}

} // namespace scene
} // namespace lux
