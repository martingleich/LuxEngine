#include "format/FormatMagicTemplates.h"
#include <initializer_list>

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
	template <>
	inline int GetAsInt(const null_type&)
	{
		return 0;
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
			return GetAsInt(*m_Data);
		}

	private:
		const T* m_Data;
	};

	using BaseFormatEntryType = char[sizeof(RefFormatEntry<null_type>)];

	template <typename T>
	void AddEntry(const T& arg, void*& ptr)
	{
		static_assert(sizeof(RefFormatEntry<T>) == sizeof(BaseFormatEntryType), "Big problem");

		new (ptr) RefFormatEntry<T>(&arg);
		ptr = (char*)ptr + sizeof(RefFormatEntry<T>);
	}

	FORMAT_API void format(Context& ctx, const BaseFormatEntryType* entries, int entryCount);

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
inline void vformat(Context& ctx, const char* str, const Types&... args)
{
	Context::AutoRestoreSubContext subCtx(ctx, str);

	// Put each argument in an array
	internal::BaseFormatEntryType entries[sizeof...(Types) ? sizeof...(Types) : 1];
	void* ptr = entries;
	(void)ptr; // Fixed warning if entryCount is zero.
	(void)std::initializer_list<int> {
		(internal::AddEntry(args, ptr), 0)...
	};

	internal::format(ctx, entries, (int)sizeof...(Types));
}

template <typename SinkT, typename... Types>
inline size_t formatEx(SinkT&& sink, const FormatExData& exData, const char* str, const Types&... args)
{
	using CleanSinkT = typename internal::remove_all<SinkT>::type;
	if(!str)
		return (size_t)-1;

	auto real_sink = sink_access<CleanSinkT>::Get(sink);

	Context ctx(exData.locale, exData.startCollumn, exData.startLine);

#ifdef FORMAT_NO_EXCEPTIONS
	try {
#endif
		vformat(ctx, str, args...);
		size_t outCharacters = real_sink.Write(ctx, ctx.GetFirstSlice(), exData.sinkFlags);
		if(exData.outCollum) {
			if(exData.sinkFlags & ESinkFlags::Newline)
				*exData.outCollum = 0;
			else
				*exData.outCollum = ctx.GetCollumn();
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
