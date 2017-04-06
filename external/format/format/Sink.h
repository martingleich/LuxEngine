#pragma once
#include "Context.h"

namespace format
{

struct ESinkFlags
{
	static const int Null = 0;
	static const int Flush = 1; // Flush the sink after writing the string.
	static const int Newline = 2; // End the string with a newline specific to the sink.
};

class sink
{
public:
	sink(size_t collumn = 0) :
		m_Collumn(collumn)
	{}
	virtual size_t Write(Context& ctx, const slice* firstSlice, int flags) = 0;
	virtual size_t GetCollumn() const
	{
		return m_Collumn;
	}

protected:
	size_t m_Collumn;
};

class sink_ref : public sink
{
public:
	sink_ref(sink& r) :
		m_Ref(r)
	{}
	virtual size_t Write(Context& ctx, const slice* firstSlice, int flags)
	{
		return m_Ref.Write(ctx, firstSlice, flags);
	}
	virtual size_t GetCollumn() const
	{
		return m_Ref.GetCollumn();
	}
private:
	sink& m_Ref;
};

template <typename T>
struct sink_access
{
	static sink_ref Get(T& x) { return sink_ref(x); }
};

}