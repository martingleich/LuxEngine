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
	core::HashMap<String, Group*> groups;
	core::Array<Group*> groupStack;

	u64 duration = 0;
	u64 frameStart;
};

RenderStatistics::RenderStatistics() :
	self(new SelfT)
{

}
RenderStatistics::~RenderStatistics()
{
	for(auto grp : self->groups)
		delete grp;

	delete self;
}

void RenderStatistics::AddPrimitives(u32 count)
{
	for(auto& e : self->groupStack)
		e->primitiveCounter += count;
}

void RenderStatistics::BeginFrame()
{
	self->frameStart = core::Clock::GetTicks();
	for(auto& grp : self->groups)
		grp->Begin();
	self->total.Begin();
	self->groupStack.Clear();
	self->groupStack.PushBack(&self->total);
}

void RenderStatistics::EndFrame()
{
	auto frameEnd = core::Clock::GetTicks();
	self->duration = frameEnd - self->frameStart;
	for(auto& grp : self->groups)
		grp->End();
	self->total.End();
}

u32 RenderStatistics::GetPrimitivesDrawn() const
{
	return self->total.primitives;
}

float RenderStatistics::GetDuration() const
{
	return self->duration / (float)core::Clock::TicksPerSecond();
}

void RenderStatistics::PushGroup(const char* name)
{
	auto& grp = self->groups[name];
	if(!grp)
		grp = new Group;
	self->groupStack.PushBack(grp);
}

void RenderStatistics::PopGroup()
{
	self->groupStack.PopBack();
}

const RenderStatistics::Group& RenderStatistics::GetGroup(const char* name) const
{
	auto it = self->groups.Find(name);
	if(it == self->groups.End())
		return self->empty;
	else
		return **it;
}

}
}
