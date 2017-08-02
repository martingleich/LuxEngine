#ifndef INCLUDED_RENDERSTATISTICS_H
#define INCLUDED_RENDERSTATISTICS_H
#include "core/ReferenceCounted.h"
#include "core/Clock.h"

namespace lux
{
namespace video
{

class RenderStatistics : public ReferenceCounted
{
public:
	RenderStatistics() :
		m_FPS(60),
		m_PrimitiveAverage(0),
		m_PrimitivesDrawn(0),
		m_FramesCounted(0),
		m_StartTime(0),
		m_PrimitivesCounted(0)
	{
	}

	inline u32 GetFPS() const
	{
		return m_FPS;
	}
	inline u32 GetPrimitivesDrawn() const
	{
		return m_PrimitivesDrawn;
	}
	inline u32 GetPrimitiveAverage() const
	{
		return m_PrimitiveAverage;
	}

	void AddPrimitives(u32 count)
	{
		m_PrimitivesDrawn += count;
	}

	void RegisterFrame()
	{
		const auto actualTime = core::Clock::GetTicks();
		const auto milliseconds = actualTime - m_StartTime;

		++m_FramesCounted;

		m_PrimitivesCounted += m_PrimitivesDrawn;
		m_PrimitivesDrawn = 0;

		if(milliseconds >= 1500) {
			const float InvMilli = 1000.0f / (float)milliseconds;

			m_FPS = (u32)(m_FramesCounted * InvMilli);
			m_PrimitiveAverage = (u32)(m_PrimitivesCounted * InvMilli);

			m_FramesCounted = 0;
			m_PrimitivesCounted = 0;
			m_StartTime = actualTime;
		}
	}

private:
	u32 m_FPS;
	u32 m_PrimitiveAverage;

	u32 m_PrimitivesDrawn;

	u32 m_FramesCounted;
	u64 m_StartTime;

	u32 m_PrimitivesCounted;
};

}
}

#endif
