#include "format/Exception.h"
#include <type_traits>

namespace format
{
namespace internal
{
	class FormatEntry
	{
	public:
		virtual void Convert(Context& ctx, Placeholder& placeholder) const = 0;
		virtual int AsInteger() const = 0;
	};

	struct null_type {};
	inline void fmtPrint(Context&, null_type, Placeholder&) {}

	template <typename T>
	typename std::enable_if<std::is_integral<T>::value, int>::type
		AsInteger(T arg)
	{
		if(arg > (T)INT_MAX)
			throw value_exception("Passed integer value is to big.");
		return (int)arg;
	}

	template <typename T>
	typename std::enable_if<!std::is_integral<T>::value, int>::type
		AsInteger(const T&)
	{
		throw value_exception("Passed placeholder value must be an integer.");
	}

	template <typename T>
	class RefFormatEntry : public FormatEntry
	{
	public:
		RefFormatEntry(const T* data) :
			m_Data(data)
		{
		}

		void Convert(Context& ctx, Placeholder& placeholder) const
		{
			fmtPrint(ctx, *m_Data, placeholder);
		}

		int AsInteger() const
		{
			return internal::AsInteger(*m_Data);
		}

	private:
		const T* m_Data;
	};

	using BaseFormatEntryType = char[sizeof(RefFormatEntry<null_type>)];

	template <typename T>
	void CheckRefEntryType()
	{
		static_assert(sizeof(RefFormatEntry<T>) <= sizeof(BaseFormatEntryType), "Big problem");
	}

	FORMAT_API void format(Context& ctx, const char* fmtStr, const BaseFormatEntryType* entries, int entryCount);
}

template <typename... Types>
inline void vformat(Context& ctx, const char* str, const Types&... args)
{
	// Validate RefFormatEntry sizes
	int unused1[] = {0, (internal::CheckRefEntryType<Types>(), 0)...};
	(void)unused1;

	// Allocate stack memory for RefEntries
	internal::BaseFormatEntryType entries[sizeof...(Types) ? sizeof...(Types) : 1]; // Arrays of size 0 are forbidden.
	void* ptr = entries;
	(void)ptr; // Fixed warning if entryCount is zero.
	int unused2[] = {0, 
		// Perform calls directly to reduce compile file size.
		(new (ptr) internal::RefFormatEntry<Types>(&args), ptr = (char*)ptr + sizeof(internal::BaseFormatEntryType), 0)...
	};
	(void)unused2;

	internal::format(ctx, str, entries, (int)sizeof...(Types));
}

template <typename SinkT, typename... Types>
inline size_t formatEx(SinkT&& sink, const FormatExData& exData, const char* str, const Types&... args)
{
	if(!str)
		return (size_t)-1;
#ifdef FORMAT_NO_EXCEPTIONS
	try {
#endif
		Context ctx(exData.locale, exData.startCollumn, exData.startLine);
		vformat(ctx, str, args...);

		using CleanSinkT =
			typename std::remove_cv<
			typename std::remove_reference<SinkT>::type>::type;
		auto real_sink = sink_access<CleanSinkT>::Get(sink);
		size_t outCharacters = real_sink.Write(ctx, ctx.Slices(), exData.sinkFlags);
		if(exData.outCollumn) {
			if(exData.sinkFlags & ESinkFlags::Newline)
				*exData.outCollumn = 0;
			else
				*exData.outCollumn = ctx.GetCollumn();
		}
		if(exData.outLine)
			*exData.outLine = ctx.GetLine();
#ifdef FORMAT_NO_EXCEPTIONS
	} catch(...) {
		outCharacters = (size_t)-1;
	}
#endif
	return outCharacters;
	}

template <typename SinkT, typename... Types>
inline size_t format(SinkT&& sink, const char* str, const Types&... args)
{
	FormatExData data;
	return formatEx(sink, data, str, args...);
}

template <typename SinkT, typename... Types>
inline size_t formatln(SinkT&& sink, const char* str, const Types&... args)
{
	FormatExData data;
	data.sinkFlags = ESinkFlags::Newline;
	return formatEx(sink, data, str, args...);
}

}
