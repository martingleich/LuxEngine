#include "Context.h"
#include "Sink.h"
#include <memory>

namespace format
{
size_t Context::GetCollumn() const
{
	if(m_Line == 0) {
		if(m_SinkCollumn == std::numeric_limits<size_t>::max())
			m_SinkCollumn = dstSink->GetCollumn();
		return m_SinkCollumn + m_Collumn;
	} else {
		return m_Collumn;
	}
}

}