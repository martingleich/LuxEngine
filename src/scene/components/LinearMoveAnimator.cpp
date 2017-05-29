#include "scene/components/LinearMoveAnimator.h"
#include "scene/SceneNode.h"
#include "core/ReferableRegister.h"

LUX_REGISTER_REFERABLE_CLASS(lux::scene::LinearMoveAnimator)

namespace lux
{
namespace scene
{

LinearMoveAnimator::LinearMoveAnimator()
{
	SetData(math::line3df(), 1.0f);
}

void LinearMoveAnimator::SetData(
	const math::line3df& line,
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

StrongRef<Referable> LinearMoveAnimator::Clone() const
{
	return LUX_NEW(LinearMoveAnimator)(*this);
}

core::Name LinearMoveAnimator::GetReferableSubType() const
{
	return SceneNodeComponentType::LinearMove;
}
}
}
