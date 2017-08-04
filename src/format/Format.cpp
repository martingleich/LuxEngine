#include "format/Format.h"
#include "format/ConvInternal.h"
#include "format/RAII.h"
#include <limits.h>

namespace format
{
namespace internal
{
	static bool TryReadInteger(StringType stringType, const char*& cur, int& out)
	{
		uint32_t c = GetCharacter(stringType, cur);
		if(c < '0' || c > '9')
			return false;
		AdvanceCursor(stringType, cur);

		const char* backtrack = cur;
		out = 0;

		while(c >= '0' && c <= '9') {
			if(out > (INT_MAX - (int)c + '0') / 10)
				throw syntax_exception("Integer literal is too big.");
			out *= 10;
			out += c - '0';
			backtrack = cur;
			c = AdvanceCursor(stringType, cur);
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
		uint32_t c = GetCharacter(ctx.fstrType, cur);
		int value;
		int idx = 0;
		int idx2 = 0;
		if(c == '?') {
			AdvanceCursor(ctx.fstrType, cur);
			outPlaceholder.master.SetPlaceholder();
			outPlaceholder.placeholderOrder[idx2++] = ' ';
			++outPlaceholder.subPlaceholderCount;
			c = GetCharacter(ctx.fstrType, cur);
		} else if(TryReadInteger(ctx.fstrType, cur, value)) {
			outPlaceholder.master.SetValue(value);
			c = GetCharacter(ctx.fstrType, cur);
		}
		while(c && !IsValidPlaceholder(c)) {
			Placeholder::Option* op = outPlaceholder.GetOption(c);
			if(!op)
				throw syntax_exception("unknown placeholder option", ctx.fstrPos);

			outPlaceholder.order[idx++] = (char)c;
			op->Enable();

			AdvanceCursor(ctx.fstrType, cur);
			c = GetCharacter(ctx.fstrType, cur); // Character after placeholder
			if(c == '?') {
				AdvanceCursor(ctx.fstrType, cur);
				op->SetPlaceholder();
				outPlaceholder.placeholderOrder[idx2++] = outPlaceholder.order[idx - 1];
				++outPlaceholder.subPlaceholderCount;
				c = GetCharacter(ctx.fstrType, cur);
			} else if(TryReadInteger(ctx.fstrType, cur, value)) {
				op->SetValue(value);
				c = GetCharacter(ctx.fstrType, cur);
			}
		}

		// Cursor is on first character after placeholder
		outPlaceholder.order[idx] = 0;
		outPlaceholder.placeholderOrder[idx2] = 0;
		if(IsValidPlaceholder(c))
			outPlaceholder.type = (char)c;
		else
			outPlaceholder.type = 0;

		if(c)
			AdvanceCursor(ctx.fstrType, cur);

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

		size_t counter;
		size_t newLines;
		size_t collumns;

		while(true) {
			size = 0;
			counter = 0;
			newLines = 0;
			collumns = 0;
			outPlaceholder.type = 0;

			// Advance cursor until ~ character.
			uint32_t c;
			const char* before = cur;
			const char* tmp;
			do {
				tmp = cur;
				c = AdvanceCursor(ctx.fstrType, cur);
				if(c != '~') {
					++counter;
					++collumns;
					if(c == '\n') {
						++newLines;
						collumns = 0;
					}
				}
			} while(c && c != '~');

			size = tmp - before;

			// The string ended before another placeholder.
			if(c == 0)
				break;

			// Remember offset of the current placeholder
			ctx.fstrLastArgPos = cur - ctx.fstr;

			ParsePlaceholder(ctx, cur, outPlaceholder);

			// If a argument free placeholder without subplaceholders is found.
			if(outPlaceholder.subPlaceholderCount == 0 && IsArgFreePlaceholder(outPlaceholder.type)) {
				ConvertAddString(ctx, ctx.fstrType, str, size);
				str = cur;

				TryFormatArgFree(ctx, outPlaceholder);
			} else {
				break;
			}
		}

		// Write remaining text
		if(size) {
			Cursor diff;
			diff.collumn = collumns;
			diff.count = counter;
			diff.line = newLines;
			ConvertAddString(ctx, ctx.fstrType, str, size, diff);
		}

		str = cur;

		return (outPlaceholder.type != 0);
	}

	static void AlignString(Context& ctx, Slice* prevSlice, size_t beforeLength, const Placeholder& placeholder)
	{
		// Align content
		const size_t curLength = ctx.GetCharacterCount();
		const size_t length = curLength - beforeLength;
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

			static const char* SPACES = "                                "; // 32 Spaces
			const size_t maxCount = 32;
			size_t count = (size_t)width - length;

			while(count > 0) {
				if(count >= maxCount) {
					prevSlice = ctx.InsertSlice(prevSlice, Slice(maxCount, SPACES));
					count -= maxCount;
				} else {
					prevSlice = ctx.InsertSlice(prevSlice, Slice(count, SPACES));
					return;
				}
			}
		}
	}

	static void WriteData(Context& ctx, const FormatEntry* entry, Placeholder& placeholder)
	{
		size_t beforeLength = ctx.GetCharacterCount();

		Slice* prevSlice = nullptr;
		if(placeholder.right_align)
			prevSlice = ctx.GetLastSlice();

#if defined(FORMAT_ERROR_TEXT) && defined(FORMAT_NO_EXCEPTIONS)
		try {
#endif
			entry->Convert(ctx, placeholder);
#if defined(FORMAT_ERROR_TEXT) && defined(FORMAT_NO_EXCEPTIONS)
		} catch(...) {
			ctx.AddSlice(14, "<FORMAT_ERROR>");
		}
#endif

		if(placeholder.left_align || placeholder.right_align)
			AlignString(ctx, prevSlice, beforeLength, placeholder);
	}

	void format(Context& ctx, StringType fmtStringType, const char* str, FormatEntry** entries, int entryCount)
	{
		const char* cur = str;

		const Context::SubContext subCtx = ctx.SaveSubContext();
		internal::RAII restoreOldSubContext([&ctx, &subCtx] { ctx.RestoreSubContext(subCtx); });
		ctx.fstrType = fmtStringType;
		ctx.fstrPos = 0;
		ctx.fstrLastArgPos = 0;
		ctx.fstr = str;
		ctx.argId = 0;

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
			for(int i = 0; i < pl.subPlaceholderCount; ++i) {
				char c = pl.placeholderOrder[i]; // The subplaceholder to replace
				if(curId >= entryCount)
					throw syntax_exception("Not enough arguments");
				if(c == ' ')
					pl.master.SetValue(entries[curId]->AsInteger());
				else
					pl.GetOption(c)->SetValue(entries[curId]->AsInteger());
				++curId;
			}
			if(isArgFree) {
				internal::TryFormatArgFree(ctx, pl);
			} else {
				if(curId >= entryCount)
					throw syntax_exception("Not enough arguments");
				internal::WriteData(ctx, entries[curId], pl);
				++curId;
			}
		}
	}
}
}
