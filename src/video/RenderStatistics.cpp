#include "video/RenderStatistics.h"
#include "core/Clock.h"

namespace lux
{
namespace video
{

static RenderStatistics g_Stats;
static RenderStatistics::Group EMPTY_GROUP;

RenderStatistics* RenderStatistics::Instance()
{
	return &g_Stats;
}

RenderStatistics::RenderStatistics()
{
}
RenderStatistics::~RenderStatistics()
{
}

void RenderStatistics::AddPrimitives(u32 count)
{
	for(auto& e : m_GroupStack)
		e->primitiveCounter += count;
}

void RenderStatistics::BeginFrame()
{
	m_FrameStart = core::Clock::GetTicks();
	for(auto& grp : m_Groups.Values())
		grp.Begin();
	auto& total = m_Groups.At("total");
	m_GroupStack.Clear();
	m_GroupStack.PushBack(&total);
}

void RenderStatistics::EndFrame()
{
	auto frameEnd = core::Clock::GetTicks();
	m_Duration = frameEnd - m_FrameStart;
	for(auto& grp : m_Groups.Values())
		grp.End();
}

u32 RenderStatistics::GetPrimitivesDrawn() const
{
	return m_Groups.Get("total").primitives;
}

float RenderStatistics::GetDuration() const
{
	return m_Duration.AsSeconds();
}

void RenderStatistics::PushGroup(core::StringView name)
{
	auto& grp = m_Groups.At(name, Group());
	m_GroupStack.PushBack(&grp);
}

void RenderStatistics::PopGroup()
{
	m_GroupStack.PopBack();
}

const RenderStatistics::Group& RenderStatistics::GetGroup(core::StringView name) const
{
	return m_Groups.Get(name, EMPTY_GROUP);
}

}
}
