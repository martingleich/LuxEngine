#include "scene/Animation.h"
#include "core/lxMemory.h"

LX_REGISTER_RESOURCE_CLASS("lux.resource.Animation", lux::scene::Animation);

lux::scene::AnimatedObject::SharedAnimatedValueList lux::scene::AnimatedObject::s_SharedAnimatedValues;

namespace lux
{
namespace scene
{

AnimationController::AnimationController()
{
}
AnimationController::AnimationController(AnimatedObject* o, Animation* a)
{
	Reset(o, a);
}

AnimationController::AnimationController(AnimationController&& old) :
	m_Object(std::move(old.m_Object)),
	m_Animation(std::move(old.m_Animation))
{
	m_Time = old.m_Time;
	m_IsLooping = old.m_IsLooping;
	m_IsPaused = old.m_IsPaused;
	m_IsFinished = old.m_IsFinished;

	m_Speed = old.m_Speed;
}

AnimationController& AnimationController::operator=(AnimationController&& old)
{
	m_Object = std::move(old.m_Object);
	m_Animation = std::move(old.m_Animation);
	m_Time = old.m_Time;
	m_IsLooping = old.m_IsLooping;
	m_IsPaused = old.m_IsPaused;
	m_IsFinished = old.m_IsFinished;

	m_Speed = old.m_Speed;
	return *this;
}

void AnimationController::Reset(AnimatedObject* o, Animation* a)
{
	m_Object = o;
	m_Animation = a;
	m_Time = m_Animation->GetStart();
	m_IsLooping = true;
	m_IsPaused = false;
	m_IsFinished = false;
	m_Speed = 1;
	m_Buffer = GetMaxSize();
	for(auto& t : m_Animation->Tracks())
		m_HandleCache.PushBack(o->GetAnimatedValueDesc(t->GetName(), t->GetType()).GetHandle());
}

void AnimationController::Tick(float secsPassed)
{
	if(!m_Animation || !m_Object)
		return;

	if(!m_IsPaused && !m_IsFinished) {
		if(!UpdateTimer(m_Time + secsPassed * m_Speed))
			return;
	}

	int id = 0;
	for(auto& track : m_Animation->Tracks()) {
		core::VariableAccess access(track->GetType(), m_Buffer);
		track->Evaluate(*this, access, &m_AnimationToken);
		m_Object->SetAnimatedValue(m_HandleCache[id], access);
		++id;
	}
}

int AnimationController::GetMaxSize() const
{
	int max = 0;
	for(int i = 0; i < m_Animation->GetTrackCount(); ++i) {
		const AnimationTrack* track = m_Animation->GetTrack(i);
		int s = track->GetType().GetSize();
		if(s > max)
			max = s;
	}

	return max;
}

void AnimationController::Seek(float time)
{
	UpdateTimer(time);
}

float AnimationController::GetTime() const
{
	return m_Time;
}

bool AnimationController::UpdateTimer(float newTime)
{
	m_Time = newTime;

	float start = m_Animation->GetStart();
	float end = m_Animation->GetEnd();
	float dur = m_Animation->GetDuration();
	if(dur <= 0) {
		m_IsFinished = true;
		return false;
	}
	while(m_Time > end) {
		if(m_IsLooping) {
			m_Time -= dur;
		} else {
			m_IsFinished = true;
			return false;
		}
	}

	while(m_Time < start)
		m_Time += dur;

	m_IsFinished = false;

	return true;
}

void AnimationController::Play(bool play)
{
	m_IsPaused = !play;
}

void AnimationController::Pause()
{
	m_IsPaused = true;
}

bool AnimationController::IsPaused() const
{
	return m_IsPaused;
}

void AnimationController::SetSpeed(float speed)
{
	m_Speed = speed;
}

float AnimationController::GetSpeed() const
{
	return m_Speed;
}

bool AnimationController::IsFinished() const
{
	return m_IsFinished;
}

float AnimationController::GetDuration() const
{
	return m_Animation->GetDuration();
}

Animation* AnimationController::GetAnimation()
{
	return m_Animation;
}

AnimatedObject* AnimationController::GetObject()
{
	return m_Object;
}

/////////////////////////////////////////////////////////////////////

Animation::Animation()
{
}

Animation::Animation(const core::ResourceOrigin& origin) :
	Resource(origin)
{
}

Animation::~Animation()
{
}

StrongRef<AnimationTrack> Animation::AddTrack(const core::String& valueName, Curve* curve)
{
	m_Tracks.PushBack(LUX_NEW(AnimationTrack)(valueName, curve));
	return *m_Tracks.Last();
}

StrongRef<AnimationTrack> Animation::GetTrack(int id)
{
	return m_Tracks[id];
}

void Animation::RemoveTrack(int id)
{
	m_Tracks.Erase(m_Tracks.First() + id);
}

void Animation::RemoveTrack(AnimationTrack* track)
{
	auto it = core::LinearSearch(track, m_Tracks);
	if(it == m_Tracks.End())
		return;
	m_Tracks.Erase(it);
}

int Animation::GetTrackCount() const
{
	return m_Tracks.Size();
}

core::Range<core::Array<StrongRef<AnimationTrack>>::Iterator> Animation::Tracks()
{
	return core::MakeRange(m_Tracks.First(), m_Tracks.End());
}

core::Range<core::Array<StrongRef<AnimationTrack>>::ConstIterator> Animation::Tracks() const
{
	return core::MakeRange(m_Tracks.First(), m_Tracks.End());
}

void Animation::SetStartEndAuto()
{
	float max = -math::Constants<float>::infinity();
	float min = math::Constants<float>::infinity();
	for(auto it = m_Tracks.First(); it != m_Tracks.End(); ++it) {
		float end = (*it)->GetEnd();
		float start = (*it)->GetStart();
		if(end > max)
			max = end;
		if(start < min)
			min = start;
	}
	if(std::isfinite(min))
		m_Start = min;
	else
		m_Start = 0;
	m_End = max;
}

void Animation::SetStartEnd(float start, float end)
{
	lxAssert(start <= end);
	m_Start = start;
	m_End = end;
}

float Animation::GetDuration() const
{
	return m_End - m_Start;
}

float Animation::GetStart() const
{
	return m_Start;
}

float Animation::GetEnd() const
{
	return m_End;
}

LUX_API core::Name Animation::GetReferableType() const
{
	return core::ResourceType::Animation;
}

} // namespace scene
} // namespace lux
