#include "format/Format.h"
#include "format/ConvInternal.h"
#include <climits>

namespace format
{
namespace internal
{
	static void FormatTilde(Context& ctx, const Placeholder& placeholder)
	{
		static const char* TILDES = "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"; // 32 Tildes

		const int count = placeholder.master.GetValue(0);

		if(count == 0)
			return;

		if(count < 0)
			throw invalid_placeholder_value("Number of tildes must be bigger than zero.", ctx.fstrLastArgPos, count);

		PutCount(ctx, count, TILDES, sizeof(TILDES));
	}

	static void FormatTab(Context& ctx, const Placeholder& placeholder)
	{
		if(!placeholder.master.HasValue())
			throw value_exception("Tab placeholder requires argument.");
		const int tab_stop = placeholder.master.GetValue();

		if(placeholder.hash) {
			if(tab_stop < 0)
				throw value_exception("Tab placeholder value must be bigger than 0.");

			PutSpaces(ctx, tab_stop);
		} else {
			if(tab_stop < 1)
				throw value_exception("Tab placeholder value must be bigger than 0.");
			if(tab_stop == 1)
				return;

			size_t p = ctx.GetCollumn() + 1;
			if(!placeholder.plus && p == 1)
				return;
			if(p%tab_stop == 0)
				return;

			size_t count = ((p / tab_stop + 1)*tab_stop - p);

			PutSpaces(ctx, count);
		}
	}

	static bool TryFormatArgFree(Context& ctx, Placeholder& placeholder)
	{
		switch(placeholder.type) {
		case '~':
			FormatTilde(ctx, placeholder);
			break;
		case 't':
			FormatTab(ctx, placeholder);
			break;
		default:
			return false;
		}

		return true;
	}

	static bool TryReadInteger(const char*& cur, int& out)
	{
		uint32_t c = *cur;
		if(c < '0' || c > '9')
			return false;
		++cur;

		const char* backtrack = cur;
		out = 0;

		while(c >= '0' && c <= '9') {
			if(out > (INT_MAX - (int)c + '0') / 10)
				throw syntax_exception("Integer literal is too big.");
			out *= 10;
			out += c - '0';
			backtrack = cur;
			c = *cur++;
		}

		cur = backtrack;

		return true;
	}

	// Parse placeholder from string
	/**
	Will throw a syntax_exception on invalid placeholder
	\param str The input string must be located on the first character of the placeholder, and will be moved onto the next character after it
	\param [out] outPlaceholder The parsed placeholder.
	*/
	static void ParsePlaceholder(Context& ctx, const char*& str, Placeholder& outPlaceholder)
	{
		outPlaceholder.Reset();

		const char* cur = str;
		char c = *cur;
		int value;
		int placeholderId = 0;
		int subPlaceholderId = 0;
		if(c == '?') {
			outPlaceholder.master.SetPlaceholder();
			outPlaceholder.placeholderOrder[subPlaceholderId++] = ' ';
			++cur;
		} else if(c >= '0' && c <= '9' && TryReadInteger(cur, value)) {
			outPlaceholder.master.SetValue(value);
		}
		c = *cur;

		while(c && !IsValidPlaceholder(c)) {
			Placeholder::Option* op = outPlaceholder.GetOption(c);
			if(!op)
				throw syntax_exception("unknown placeholder option", ctx.argId);

			outPlaceholder.order[placeholderId++] = (char)c;
			op->Enable();

			++cur;
			c = *cur; // Character after placeholder
			if(c == '?') {
				++cur;
				op->SetPlaceholder();
				outPlaceholder.placeholderOrder[subPlaceholderId++] = outPlaceholder.order[placeholderId - 1];
				c = *cur;
			} else if(TryReadInteger(cur, value)) {
				op->SetValue(value);
				c = *cur;
			}
		}

		// Cursor is on first character after placeholder
		outPlaceholder.order[placeholderId] = 0;
		outPlaceholder.placeholderOrder[subPlaceholderId] = 0;
		if(IsValidPlaceholder(c))
			outPlaceholder.type = (char)c;
		else
			outPlaceholder.type = 0;

		if(c)
			cur++;

		str = cur;
	}

	//! Read until placeholder.
	/**
	Will read until a placeholder is encounterd.
	\param ctx The format context.
	\param str The input string, will be put on the first character after the placeholder
	\param outPlaceholder The placeholder which was read last
	\return True if a placeholder was found, otherwise false
	*/
	static bool ParseUntilPlaceholder(
		Context& ctx,
		const char*& str,
		Placeholder& outPlaceholder)
	{
		const char* cur = str;
		size_t size;

		while(true) {
			size = 0;
			outPlaceholder.type = 0;

			// Advance cursor until ~ character.
			const char* before = cur;
			while(*cur && *cur != '~')
				++cur;
			size = cur - before;

			// The string ended before another placeholder.
			if(*cur == 0)
				break;

			// Move to first character of placeholder
			++cur;

			// Remember offset of the current placeholder
			ctx.fstrLastArgPos = cur - ctx.GetFormatString();
			++ctx.argId;

			ParsePlaceholder(ctx, cur, outPlaceholder);

			// If a argument free placeholder without subplaceholders is found.
			if(!outPlaceholder.HasSubPlaceholder() && IsArgFreePlaceholder(outPlaceholder.type)) {
				ctx.AddSlice(size, str);
				TryFormatArgFree(ctx, outPlaceholder);
			} else {
				break;
			}
		}

		// Write remaining text
		if(size)
			ctx.AddSlice(size, str);

		str = cur;
		return (outPlaceholder.type != 0);
	}

	static void AlignString(Context& ctx, Slice* rightAlignSlice, size_t length, const Placeholder& placeholder)
	{
		static const char* SPACES = "                                "; // 32 Spaces

		// Align content
		if(placeholder.left_align) {
			if(!placeholder.left_align.HasValue())
				return;

			int width = placeholder.left_align.GetValue(0);
			if(width < 0)
				throw syntax_exception("Width of align must be positive.");
			if((size_t)width <= length)
				return;

			const size_t count = (size_t)width - length;
			PutSpaces(ctx, count);
		} else if(placeholder.right_align) {
			if(!placeholder.right_align.HasValue())
				return;

			int width = placeholder.right_align.GetValue(0);
			if(width < 0)
				throw syntax_exception("Width of align must be positive.");
			if((size_t)width <= length)
				return;

			const size_t maxCount = 32;
			size_t count = (size_t)width - length;
			if(count > maxCount)
				throw syntax_exception("Width of align must be smaller than 32.");
			rightAlignSlice->data = SPACES;
			rightAlignSlice->size = count;
		}
	}

	static void WriteData(Context& ctx, const FormatEntry* entry, Placeholder& placeholder)
	{
		size_t beforeLength = 0;
		Slice* alignSlice = nullptr;
		if(placeholder.left_align || placeholder.right_align) {
			beforeLength = ctx.StartCounting();
			if(placeholder.right_align)
				alignSlice = ctx.AddLockedSlice();
		}

#if defined(FORMAT_ERROR_TEXT) && defined(FORMAT_NO_EXCEPTIONS)
		try {
#endif
			entry->Convert(ctx, placeholder);
#if defined(FORMAT_ERROR_TEXT) && defined(FORMAT_NO_EXCEPTIONS)
		} catch(format_exception&) {
			ctx.AddTermiatedSlice("<FORMAT_ERROR>");
		}
#endif

		if(placeholder.left_align || placeholder.right_align) {
			auto newLength = ctx.StopCounting();
			AlignString(ctx, alignSlice, newLength - beforeLength - 1, placeholder);
		}
	}

	void format(Context& ctx, const char* fmtStr, const BaseFormatEntryType* rawEntries, int entryCount)
	{
		Context::AutoRestoreSubContext subCtx(ctx, fmtStr);

		auto GetEntry = [&](int i) { return reinterpret_cast<const FormatEntry*>(rawEntries + i); };
		const char* cur = ctx.GetFormatString();

		// Go to placeholder, try arg free, fill subplaceholder, convert string
		int curId = 0;
		Placeholder pl;
		while(true) {
			if(!internal::ParseUntilPlaceholder(ctx, cur, pl))
				break; // No more placeholders
			bool isArgFree = IsArgFreePlaceholder(pl.type);
			if(!isArgFree)
				curId = pl.master.GetValue(curId);

			// Replace subplaceholders
			for(char* sub = pl.placeholderOrder; *sub; ++sub) {
				if(curId >= entryCount)
					throw syntax_exception("Not enough arguments");
				if(*sub == ' ')
					pl.master.SetValue(GetEntry(curId)->AsInteger());
				else
					pl.GetOption(*sub)->SetValue(GetEntry(curId)->AsInteger());
				++curId;
			}
			if(isArgFree) {
				internal::TryFormatArgFree(ctx, pl);
			} else {
				if(curId >= entryCount)
					throw syntax_exception("Not enough arguments");
				internal::WriteData(ctx, GetEntry(curId), pl);
				++curId;
			}
		}
	}
}
}
