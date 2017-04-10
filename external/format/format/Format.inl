#pragma once
#include "RAII.h"
#include "ConvInternal.h"

namespace format
{
class Context;
namespace internal
{
	struct PlaceholderCtx;

	const char* ParseUntilPlaceholder(Context& ctx, const char* str, Placeholder& outPlaceholder);
	void ReplaceSubPlaceholder(PlaceholderCtx& ctx, int argInt);
	void AlignString(Context& ctx, slice* prevSlice, size_t beforeLength, const Placeholder& placeholder);

	template <typename T>
	bool UpdateSubPlaceholders(
		PlaceholderCtx& placeholderCtx,
		Context& ctx,
		const T& arg,
		const char*& cur);

	template <typename T>
	void WriteData(Context& ctx, const T& arg, Placeholder& placeholder);

	template <typename T>
	void format(
		PlaceholderCtx& placeholderCtx,
		Context& ctx,
		const T& arg,
		const char*& cur);
}

namespace internal
{
	struct PlaceholderCtx
	{
		int subId; // The index of the subplaceholder zu replace next.
		int ignoreCount; // How many subplaceholders remain.
		Placeholder placeholder;
		bool active;

		PlaceholderCtx() :
			subId(0),
			ignoreCount(0),
			active(false)
		{
		}
	};

	//! Update subplaceholders
	/**
	Replaces possible subplaceholder with the passed argument.
	Advances cursor until next placeholder if possible.
	\param arg The argument to insert.
	\param inout cur The cursor into the format string
	\return True if the argument was consumed otherwise false.
	*/
	template <typename T>
	inline bool UpdateSubPlaceholders(
		PlaceholderCtx& placeholderCtx,
		Context& ctx,
		const T& arg,
		const char*& cur)
	{
		if(placeholderCtx.ignoreCount > 0) {
			ReplaceSubPlaceholder(placeholderCtx, GetAsInt(arg));
			return true;
		}

		if(!placeholderCtx.active) {
			cur = ParseUntilPlaceholder(ctx, cur, placeholderCtx.placeholder);
			if(placeholderCtx.placeholder.type == 0)
				throw syntax_exception();

			placeholderCtx.active = true;

			if(placeholderCtx.placeholder.subPlaceholderCount) {
				placeholderCtx.ignoreCount = placeholderCtx.placeholder.subPlaceholderCount;
				placeholderCtx.subId = 0;
				ReplaceSubPlaceholder(placeholderCtx, GetAsInt(arg));
				return true;
			}
		}

		return false;
	}

	template <typename T>
	inline void WriteData(Context& ctx, const T& arg, Placeholder& placeholder)
	{
		size_t beforeLength = ctx.GetCharacterCount();

		slice* prevSlice = nullptr;
		if(placeholder.right_align)
			prevSlice = ctx.GetLastSlice();

#if defined(FORMAT_ERROR_TEXT) && defined(FORMAT_NO_EXCEPTIONS)
		try {
#endif
			conv_data(ctx, arg, placeholder);
#if defined(FORMAT_ERROR_TEXT) && defined(FORMAT_NO_EXCEPTIONS)
		} catch(...) {
			ctx.AddSlice(14, "<FORMAT_ERROR>");
		}
#endif

		if(placeholder.left_align || placeholder.right_align)
			AlignString(ctx, prevSlice, beforeLength, placeholder);
	}

	// Format a part of input data
	/**
	Will consume input until the next placeholder and replace it with the argument.
	Will throw exceptions on bad syntax.
	Can throw exception on conversion error, depending on FORMAT_ERROR_TEXT.
	\param ctx The format context.
	\param arg The argument to insert into the placeholder
	\param [inout] The cursor into the format string.
	*/
	template <typename T>
	inline void format(
		PlaceholderCtx& placeholderCtx,
		Context& ctx,
		const T& arg,
		const char*& cur)
	{
		ctx.argId++;

		bool consumed = UpdateSubPlaceholders(placeholderCtx, ctx, arg, cur);
		if(placeholderCtx.ignoreCount == 0) {
			if(consumed) {
				// Can only be argument free, since the argument was consumed.
				if(TryFormatArgFree(ctx, placeholderCtx.placeholder))
					placeholderCtx.active = false;
			} else {
				WriteData(ctx, arg, placeholderCtx.placeholder);
				placeholderCtx.active = false;
			}
		}
	}

}

// Format string into Context
/**
Will throw syntax_exception on bad syntax.
Can throw exception on conversion error, depending on FORMAT_ERROR_TEXT.
Usefull conv_data implementations.
Will perform normal formatting, but will write the result into a Context.
*/
template <typename... Types>
inline void vformat(Context& ctx, StringType fmtStringType, const char* str, const Types&... args)
{
	const char* cur = str;

	const Context::SubContext subCtx = ctx.SaveSubContext();
	internal::RAII restoreOldSubContext([&ctx, &subCtx] { ctx.RestoreSubContext(subCtx); });
	ctx.fstrType = fmtStringType;
	ctx.fstrPos = 0;
	ctx.fstrLastArgPos = 0;
	ctx.fstr = str;
	ctx.argId = 0;

	// Call internal::format for every argument.
	// I don't know a other nicer way to do that than the temporary initializer_list
	// This list is normally fully removed by the optimizer.
	internal::PlaceholderCtx placeholderCtx;
	(void)std::initializer_list<int>{
		(internal::format(
			placeholderCtx,
			ctx,
			internal::format_type_conv<Types>::get(args),
			cur), 0)...
	};

	if(placeholderCtx.ignoreCount != 0)
		throw syntax_exception("Missing arguments for placeholder.", ctx.fstrLastArgPos);
	// cur now points to the element after the last placeholder.

	if(GetCharacter(ctx.stringType, cur)) { // Parse the rest of the input which contains
		cur = internal::ParseUntilPlaceholder(ctx, cur, placeholderCtx.placeholder);
		if(placeholderCtx.placeholder.type != 0) // placeholder without passed value.
			throw syntax_exception("Missing arguments for placeholder.", ctx.fstrLastArgPos);
	}
}

template <typename SinkT, typename... Types>
inline size_t formatEx(SinkT&& sink, StringType dstStringType, const locale::Locale* locale, int sinkFlags, StringType fmtStringType, const char* str, const Types&... args)
{
	typedef std::remove_const<typename std::remove_cv<typename std::remove_reference<SinkT>::type>::type>::type CleanSinkT;
	if(!str)
		return (size_t)-1;

	auto real_sink = sink_access<CleanSinkT>::Get(sink);

	Context ctx;
	ctx.SetDstStringType(dstStringType);
	ctx.SetFmtStringType(fmtStringType);
	ctx.SetLocale(locale ? locale : locale::GetLocale());
	ctx.dstSink = &real_sink;
	internal::RAII clearMemory_fence([&ctx] { ctx.ClearMemory(); });

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
inline size_t format(SinkT&& sink, const char* str, const Types&... args)
{
	return formatEx(sink, StringType::FORMAT_STRING_TYPE, nullptr, ESinkFlags::Null, StringType::FORMAT_STRING_TYPE, str, args...);
}

template <typename SinkT, typename... Types>
inline size_t formatln(SinkT&& sink, const char* str, const Types&... args)
{
	return formatEx(sink, StringType::FORMAT_STRING_TYPE, nullptr, ESinkFlags::Newline | ESinkFlags::Flush, StringType::FORMAT_STRING_TYPE, str, args...);
}

}