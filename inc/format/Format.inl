#include "format/Exception.h"
#include <type_traits>

namespace format
{
namespace internal
{
	struct null_type {};
	inline void fmtPrint(Context&, null_type, Placeholder&) {}

	template <typename T>
	typename std::enable_if<std::is_integral<T>::value, int>::type
		AsInteger(T arg)
	{
		if(arg > (T)INT_MAX)
			throw invalid_argument("Passed integer value is to big.", size_t(-1));
		return (int)arg;
	}

	template <typename T>
	typename std::enable_if<!std::is_integral<T>::value, int>::type
		AsInteger(const T&)
	{
		throw invalid_argument("Passed placeholder value must be an integer.", size_t(-1));
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
		static_assert(sizeof(RefFormatEntry<T>) == sizeof(BaseFormatEntryType), "Big problem");
	}

	FORMAT_API void format(Context& ctx, Slice fmtStr);
}

template <typename... Types>
inline void vformat(Context& ctx, Slice str, const Types&... args)
{
	// Validate RefFormatEntry sizes
	int unused1[] = {0, (internal::CheckRefEntryType<Types>(), 0)...};
	(void)unused1;

	// Allocate stack memory for RefEntries
	internal::BaseFormatEntryType entries[sizeof...(Types) ? sizeof...(Types) : 1]; // Arrays of size 0 are forbidden.
	char* ptr = (char*)entries;
	(void)ptr; // Fixed warning if entryCount is zero.
	int unused2[] = {0, 
		// Perform calls directly to reduce compile file size.
		(new (ptr) internal::RefFormatEntry<Types>(&args), ptr += sizeof(internal::BaseFormatEntryType), 0)...
	};
	(void)unused2;

	ctx.SetFormatEntries(entries, (int)sizeof...(Types), sizeof(internal::BaseFormatEntryType));
	internal::format(ctx, str);
}

template <typename... Types>
inline void vformat(Context& ctx, const char* str, const Types&... args)
{
	vformat(ctx, Slice(std::strlen(str), str), args...);
}

template <typename SinkT, typename... Types>
inline int formatEx(SinkT&& sink, const FormatExData& exData, Slice str, const Types&... args)
{
	if(!str.data)
		return -1;
#ifdef FORMAT_NO_EXCEPTIONS
	try {
#endif
		Context ctx(exData.locale ? *exData.locale : *GetLocale());
		vformat(ctx, str, args...);

		using CleanSinkT =
			typename std::remove_cv<
			typename std::remove_reference<SinkT>::type>::type;
		auto real_sink = sink_access<CleanSinkT>::Get(sink);
		return (int)real_sink.Write(ctx, ctx.Slices(), exData.sinkFlags);
#ifdef FORMAT_NO_EXCEPTIONS
	} catch(...) {
		return -1;
	}
#endif
}

template <typename SinkT, typename... Types>
inline int format(SinkT&& sink, const char* str, const Types&... args)
{
	FormatExData data;
	return formatEx(sink, data, Slice(std::strlen(str), str), args...);
}
template <typename SinkT, typename... Types>
inline int format(SinkT&& sink, Locale* locale, const char* str, const Types&... args)
{
	FormatExData data;
	data.locale = locale;
	return formatEx(sink, data, Slice(std::strlen(str), str), args...);
}
template <typename SinkT, typename... Types>
inline int format(SinkT&& sink, Slice str, const Types&... args)
{
	FormatExData data;
	return formatEx(sink, data, str, args...);
}
template <typename SinkT, typename... Types>
inline int formatln(SinkT&& sink, const char* str, const Types&... args)
{
	return formatln(sink, Slice(std::strlen(str), str), args...);
}
template <typename SinkT, typename... Types>
inline int formatln(SinkT&& sink, Slice str, const Types&... args)
{
	FormatExData data;
	data.sinkFlags = ESinkFlags::Newline;
	return formatEx(sink, data, str, args...);
}

}
