#ifndef INCLUDED_ANIMATIONS
#define INCLUDED_ANIMATIONS
#include "core/Resource.h"
#include "scene/Curve.h"

namespace lux
{
namespace scene
{

class AnimatedValueHandle
{
public:
	bool operator==(AnimatedValueHandle other) const { return id == other.id; }
	bool operator!=(AnimatedValueHandle other) const { return id != other.id; }
	int id;
};

class AnimatedValueDesc
{
public:
	AnimatedValueDesc(const core::String& n, core::Type t, AnimatedValueHandle h) :
		name(n),
		type(t),
		handle(h)
	{
	}
	const core::String& GetName() const { return name; }
	core::Type GetType() const { return type; }
	AnimatedValueHandle GetHandle() const { return handle; }

private:
	core::String name;
	core::Type type;
	AnimatedValueHandle handle;
};

class AnimatedObject : public virtual ReferenceCounted
{
	class DescIter : public core::BaseIterator<core::ForwardIteratorTag, AnimatedValueDesc>
	{
		friend class AnimatedObject;
	public:
		bool operator==(DescIter other) const { return ref == other.ref; }
		bool operator!=(DescIter other) const { return ref != other.ref; }
		DescIter& operator++()
		{
			ref++;
			if(ref == end)
				ref = begin2;
			return *this;
		}
		DescIter operator++(int) { DescIter tmp(*this); ref++; return tmp; }
		const AnimatedValueDesc& operator*() const { return *ref; }
	private:
		DescIter(
			const AnimatedValueDesc* _ref,
			const AnimatedValueDesc* _end,
			const AnimatedValueDesc* _begin2) :
			ref(_ref),
			end(_end),
			begin2(_begin2)
		{
		}
		const AnimatedValueDesc* ref;
		const AnimatedValueDesc* end = nullptr;
		const AnimatedValueDesc* begin2 = nullptr;
	};
	class SharedAnimatedValueList
	{
	public:
		core::Array<AnimatedValueDesc> members;
		bool isInitialized = false;
	};

public:
	virtual const AnimatedValueDesc GetAnimatedValueDesc(const core::String& name, const core::Type& type = core::Type::Unknown)
	{
		for(auto x : GetAnimatedValueDescs()) {
			if(x.GetName() == name) {
				if(type != core::Type::Unknown && type != x.GetType())
					break;
				return x;
			}
		}
		throw core::ObjectNotFoundException(name.Data());
	}
	virtual core::Range<DescIter> GetAnimatedValueDescs() const
	{
		return MakeDescList(nullptr, nullptr);
	}

	virtual void SetAnimatedValue(AnimatedValueHandle handle, const core::VariableAccess& data) = 0;
	virtual void GetAnimatedValue(AnimatedValueHandle handle, const core::VariableAccess& data) = 0;

protected:
	virtual void InitSharedAnimatedValues() const {}
	static AnimatedValueHandle AddSharedAnimatedValue(const char* name, core::Type type, int forceId = -1)
	{
		if(s_SharedAnimatedValues.isInitialized)
			throw core::InvalidOperationException("Can't change animated values, after first call of InitAnimatedValues().");
		if(forceId < 0)
			forceId = s_SharedAnimatedValues.members.Size();
		auto newHandle = AnimatedValueHandle{forceId};
		s_SharedAnimatedValues.members.EmplaceBack(name, type, newHandle);
		return newHandle;
	}

	core::Range<DescIter> MakeDescList(
		const AnimatedValueDesc* begin,
		const AnimatedValueDesc* end) const
	{
		if(!s_SharedAnimatedValues.isInitialized) {
			InitSharedAnimatedValues();
			s_SharedAnimatedValues.isInitialized = true;
		}
		auto sharedBegin = s_SharedAnimatedValues.members.Data();
		auto sharedEnd = sharedBegin + s_SharedAnimatedValues.members.Size();
		return core::MakeRange(
			DescIter(sharedBegin, sharedEnd, begin),
			DescIter(end, sharedEnd, begin));
	}

private:
	LUX_API static SharedAnimatedValueList s_SharedAnimatedValues;
};

class Animation;
class AnimationController
{
public:
	LUX_API AnimationController();
	LUX_API AnimationController(AnimatedObject* o, Animation* a);
	LUX_API void Reset(AnimatedObject* o, Animation* a);
	AnimationController(const AnimationController& old) = delete;
	LUX_API AnimationController(AnimationController&& old);
	AnimationController& operator=(const AnimationController& old) = delete;
	LUX_API AnimationController& operator=(AnimationController&& old);

	LUX_API void Tick(float secsPassed);
	LUX_API void Seek(float time);
	LUX_API float GetTime() const;

	LUX_API void Play(bool play = false);
	LUX_API void Pause();
	LUX_API bool IsPaused() const;
	LUX_API void SetSpeed(float speed);
	LUX_API float GetSpeed() const;

	LUX_API bool IsFinished() const;

	LUX_API float GetDuration() const;

	LUX_API Animation* GetAnimation();
	LUX_API AnimatedObject* GetObject();

private:
	int GetMaxSize() const;
	bool UpdateTimer(float newTime);

private:
	WeakRef<AnimatedObject> m_Object;
	StrongRef<Animation> m_Animation;

	core::Array<AnimatedValueHandle> m_HandleCache;

	core::RawMemory m_Buffer;

	u32 m_AnimationToken;

	float m_Time;
	float m_Speed;

	bool m_IsLooping : 1;
	bool m_IsPaused : 1;
	bool m_IsFinished : 1;
};

class AnimationTrack : public ReferenceCounted
{
public:
	AnimationTrack(const core::String& valueName, Curve* curve) :
		m_ValueName(valueName),
		m_Curve(curve)
	{
		m_Type = m_Curve->GetType();
	}

	void Evaluate(const AnimationController& playback, const core::VariableAccess& access, u32* token=nullptr) const
	{
		m_Curve->Evaluate(playback.GetTime(), access, token);
	}

	core::Type GetType() const { return m_Type; } 
	const core::String& GetName() const { return m_ValueName; }
	StrongRef<Curve> GetCurve() { return m_Curve; }
	float GetStart() const { return m_Curve->GetStart(); }
	float GetEnd() const { return m_Curve->GetEnd(); }

private:
	core::Type m_Type;
	StrongRef<Curve> m_Curve;
	core::String m_ValueName;
};

class Animation : public core::Resource
{
	using TrackIterT = core::Array<StrongRef<AnimationTrack>>::Iterator;
	using ConstTrackIterT = core::Array<StrongRef<AnimationTrack>>::ConstIterator;
public:
	LUX_API Animation();
	LUX_API Animation(const core::ResourceOrigin& origin);
	LUX_API ~Animation();

	LUX_API StrongRef<AnimationTrack> AddTrack(const core::String& valueName, Curve* curve);
	LUX_API StrongRef<AnimationTrack> GetTrack(int id);
	LUX_API void RemoveTrack(int id);
	LUX_API void RemoveTrack(AnimationTrack* track);
	LUX_API int GetTrackCount() const;
	LUX_API core::Range<TrackIterT> Tracks();
	LUX_API core::Range<ConstTrackIterT> Tracks() const;
	LUX_API void SetStartEndAuto();
	LUX_API void SetStartEnd(float start, float end);
	LUX_API float GetDuration() const;
	LUX_API float GetStart() const;
	LUX_API float GetEnd() const;

	LUX_API core::Name GetReferableType() const;

private:
	core::Array<StrongRef<AnimationTrack>> m_Tracks;
	float m_Start;
	float m_End;
};

} // namespace scene
} // namespace lux

#endif // #ifndef INCLUDED_ANIMATIONS