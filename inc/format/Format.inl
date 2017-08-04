#include "FormatMagicTemplates.h"
#include <initializer_list>
#include <type_traits>

namespace format
{
namespace internal
{
	template <typename T1, typename T2, bool First>
	struct Select;

	template <typename T1, typename T2>
	struct Select<T1, T2, true> { using type = T1; };

	template <typename T1, typename T2>
	struct Select<T1, T2, false> { using type = T2; };

	class FormatEntry
	{
	public:
		virtual ~FormatEntry() {}
		virtual void Convert(Context& ctx, Placeholder& placeholder) const = 0;
		virtual int AsInteger() const = 0;
	};

	template <typename T>
	class RefFormatEntry : public FormatEntry
	{
	public:
		RefFormatEntry(const T& data) :
			m_Data(data)
		{
		}

		void Convert(Context& ctx, Placeholder& placeholder) const
		{
			conv_data(ctx, m_Data, placeholder);
		}

		int AsInteger() const
		{
			return GetAsInt(m_Data);
		}

	private:
		const T& m_Data;
	};

	template <typename T>
	class CopyFormatEntry : public FormatEntry
	{
	public:
		CopyFormatEntry(const T& data) :
			m_Data(data)
		{
		}

		void Convert(Context& ctx, Placeholder& placeholder) const
		{
			conv_data(ctx, m_Data, placeholder);
		}

		int AsInteger() const
		{
			return GetAsInt(m_Data);
		}

	private:
		T m_Data;
	};

	template <typename T>
	void AddEntry(const T& arg, FormatEntry**& ptr)
	{
		*ptr++ = new typename Select<
			CopyFormatEntry<T>,
			RefFormatEntry<T>,
			std::is_integral<T>::value || std::is_floating_point<T>::value || std::is_pointer<T>::value
		>::type(arg);
	}

	LUX_API void format(Context& ctx, StringType fmtStringType, const char* str, FormatEntry** entries, int entryCount);

	template<class T> struct remove_all { typedef T type; };
	template<class T> struct remove_all<T&> : remove_all<T> {};
	template<class T> struct remove_all<T const> : remove_all<T> {};
	template<class T> struct remove_all<T volatile> : remove_all<T> {};
	template<class T> struct remove_all<T const volatile> : remove_all<T> {};
}

// Format string into Context
/**
Will throw syntax_exception on bad syntax.
Can throw exception on conversion error, depending on FORMAT_ERROR_TEXT.
Useful conv_data implementations.
Will perform normal formatting, but will write the result into a Context.
*/
template <typename... Types>
inline void vformat(Context& ctx, StringType fmtStringType, const char* str, const Types&... args)
{
	// Put each argument in an array
	struct Entries
	{
		const int entryCount = (int)sizeof...(Types);
		internal::FormatEntry* entries[sizeof...(Types) ? sizeof...(Types) : 1];

		Entries(const Types&... args)
		{
			auto ptr = entries;
			(void)ptr; // Fixed warning if entryCount is zero.
			(void)std::initializer_list<int> {
				(internal::AddEntry(internal::format_type_conv<Types>::get(args), ptr), 0)...
			};
		}

		~Entries()
		{
			for(int i = 0; i < entryCount; ++i)
				delete entries[i];
		}
	};

	Entries entries(args...);
	internal::format(ctx, fmtStringType, str, entries.entries, entries.entryCount);
}

template <typename SinkT, typename... Types>
inline size_t formatEx(SinkT&& Sink, StringType dstStringType, const Locale* locale, int sinkFlags, StringType fmtStringType, const char* str, const Types&... args)
{
	using CleanSinkT = typename internal::remove_all<SinkT>::type;
	if(!str)
		return (size_t)-1;

	auto real_sink = sink_access<CleanSinkT>::Get(Sink);

	Context ctx;
	ctx.SetDstStringType(dstStringType);
	ctx.SetFmtStringType(fmtStringType);
	ctx.SetLocale(locale ? locale : GetLocale());
	ctx.dstSink = &real_sink;

#ifdef FORMAT_NO_EXCEPTIONS
	try {
#endif
		vformat(ctx, fmtStringType, str, args...);
		return real_sink.Write(ctx, ctx.GetFirstSlice(), sinkFlags);
#ifdef FORMAT_NO_EXCEPTIONS
	} catch(...) {
		return (size_t)-1;
	}
#endif
}

template <typename SinkT, typename... Types>
inline size_t format(SinkT&& Sink, const char* str, const Types&... args)
{
	return formatEx(Sink, StringType::FORMAT_STRING_TYPE, nullptr, ESinkFlags::Null, StringType::FORMAT_STRING_TYPE, str, args...);
}

template <typename SinkT, typename... Types>
inline size_t formatln(SinkT&& Sink, const char* str, const Types&... args)
{
	return formatEx(Sink, StringType::FORMAT_STRING_TYPE, nullptr, ESinkFlags::Newline | ESinkFlags::Flush, StringType::FORMAT_STRING_TYPE, str, args...);
}

}
