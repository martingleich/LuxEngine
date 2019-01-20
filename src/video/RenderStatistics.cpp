#include "video/RenderStatistics.h"
#include "core/Clock.h"
#include "core/lxArray.h"
#include "core/lxString.h"
#include "core/lxHashMap.h"

namespace lux
{
namespace video
{
static StrongRef<RenderStatistics> g_Stats;
RenderStatistics* RenderStatistics::Instance()
{
	if(!g_Stats)
		g_Stats = LUX_NEW(RenderStatistics);
	return g_Stats;
}

struct RenderStatistics::SelfT
{
	Group empty;
	Group total;
	core::HashMap<core::String, Group*> groups;
	core::Array<Group*> groupStack;

	core::Duration duration;
	core::Duration frameStart;
};

RenderStatistics::RenderStatistics() :
	self(LUX_NEW(SelfT))
{

}
RenderStatistics::~RenderStatistics()
{
	for(auto grp : self->groups.Values())
		LUX_FREE(grp);

	LUX_FREE(self);
}

void RenderStatistics::AddPrimitives(u32 count)
{
	for(auto& e : self->groupStack)
		e->primitiveCounter += count;
}

void RenderStatistics::BeginFrame()
{
	self->frameStart = core::Clock::GetTicks();
	for(auto& grp : self->groups.Values())
		grp->Begin();
	self->total.Begin();
	self->groupStack.Clear();
	self->groupStack.PushBack(&self->total);
}

void RenderStatistics::EndFrame()
{
	auto frameEnd = core::Clock::GetTicks();
	self->duration = frameEnd - self->frameStart;
	for(auto& grp : self->groups.Values())
		grp->End();
	self->total.End();
}

u32 RenderStatistics::GetPrimitivesDrawn() const
{
	return self->total.primitives;
}

float RenderStatistics::GetDuration() const
{
	return self->duration.AsSeconds();
}

void RenderStatistics::PushGroup(const char* name)
{
	auto& grp = self->groups.At(name, nullptr);
	if(!grp)
		grp = LUX_NEW(Group);
	self->groupStack.PushBack(grp);
}

void RenderStatistics::PopGroup()
{
	self->groupStack.PopBack();
}

const RenderStatistics::Group& RenderStatistics::GetGroup(const char* name) const
{
	auto it = self->groups.Find<const char*>(name);
	if(it == self->groups.end())
		return self->empty;
	else
		return *it->value;
}

}
}
