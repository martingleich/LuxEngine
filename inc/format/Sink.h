#ifndef INCLUDED_FORMAT_SINK_H
#define INCLUDED_FORMAT_SINK_H

#include "format/Context.h"

namespace format
{

//! Flags to changed the behaivor of the sink
struct ESinkFlags
{
	static const int Null = 0; //!< No sinkflags used
	static const int Flush = 1; //!< Flush the sink after writing the string.
	static const int Newline = 2; //!< End the string with a newline specific to the sink.
};

//! Sinks are targets where data from format command is written to
class Sink
{
public:
	virtual ~Sink() {}

	//! Write data to the sink.
	/**
	This function is only called once for each format call, all the passed data should be written at once to the output
	\param ctx The context used to write the data, contains information about string types and so on
	\param firstSlice The firstSlice to write to the output, the next slice can be accessed through it
	\param flags The flags to used while writing
	\return The number of characters written
	*/
	virtual size_t Write(Context& ctx, const Slice* firstSlice, int flags) = 0;
};

//! Wrapper around a sink reference
/**
This class is used to access sink references
*/
class sink_ref : public Sink
{
public:
	sink_ref(Sink& r) :
		m_Ref(r)
	{
	}
	virtual size_t Write(Context& ctx, const Slice* firstSlice, int flags)
	{
		return m_Ref.Write(ctx, firstSlice, flags);
	}
private:
	Sink& m_Ref;
};

//! The sink_access wrapper.
/**
All classes used as sink-types must implement a sink_access specialication
The default implemention handles all classes inheriting from format::Sink.
You can use classes like FILE*, std::ostream, or std::string as sinks, by
specialicing this class, and returning a custom sink class.
See SinkOStream, SinkCFil, or SinkStdString for examples of usage.
*/
template <typename T>
struct sink_access
{
	//! Sink access function
	/**
	Must return a class inheriting from format::Sink
	*/
	static sink_ref Get(T& x) { return sink_ref(x); }
};

}
#endif // #ifndef INCLUDED_FORMAT_SINK_H
