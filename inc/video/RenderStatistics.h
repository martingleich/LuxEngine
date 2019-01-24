#ifndef INCLUDED_LUX_RENDERSTATISTICS_H
#define INCLUDED_LUX_RENDERSTATISTICS_H
#include "core/ReferenceCounted.h"
#include "core/lxArray.h"
#include "core/lxString.h"
#include "core/lxHashMap.h"

namespace lux
{
namespace video
{

class RenderStatistics : public ReferenceCounted
{
public:
	struct Group
	{
		u32 primitiveCounter = 0;
		u32 primitives = 0;

		void Begin()
		{
			primitiveCounter = 0;
		}

		void End()
		{
			primitives = primitiveCounter;
		}
	};

	struct GroupScope
	{
		GroupScope(core::StringView name)
		{
			RenderStatistics::Instance()->PushGroup(name);
		}

		~GroupScope()
		{
			RenderStatistics::Instance()->PopGroup();
		}
	};

public:
	LUX_API static RenderStatistics* Instance();

	LUX_API RenderStatistics();
	LUX_API ~RenderStatistics();
	LUX_API void AddPrimitives(u32 count);
	LUX_API void BeginFrame();
	LUX_API void EndFrame();
	LUX_API u32 GetPrimitivesDrawn() const;
	LUX_API float GetDuration() const;
	LUX_API void PushGroup(core::StringView name);
	LUX_API void PopGroup();
	LUX_API const Group& GetGroup(core::StringView name) const;

private:
	core::HashMap<core::String, Group> m_Groups;
	core::Array<Group*> m_GroupStack;

	core::Duration m_Duration;
	core::Duration m_FrameStart;
};

}
}

#endif
