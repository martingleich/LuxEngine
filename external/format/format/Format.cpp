#include "Format.h"
#include "Context.h"
#include <limits>

namespace format
{
namespace internal
{
	bool TryReadInteger(StringType stringType, const char*& cur, int& out)
	{
		uint32_t c = GetCharacter(stringType, cur);
		if(c < '0' || c > '9')
			return false;
		AdvanceCursor(stringType, cur);

		const char* backtrack = cur;
		out = 0;
		while(c >= '0' && c <= '9') {
			if(out > (std::numeric_limits<int>::max() - (int)c + '0') / 10)
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
	\param str The input string must be located on the first character of the placeholder.
	\param [out] outPlaceholder The parsed placeholder.
	\return The first character after the placeholder.
	*/
	const char* ParsePlaceholder(Context& ctx, const char* str, Placeholder& outPlaceholder, int& subPlaceholderCount)
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
			++subPlaceholderCount;
			c = GetCharacter(ctx.fstrType, cur);
		} else if(TryReadInteger(ctx.fstrType, cur, value)) {
			outPlaceholder.master.SetValue(value);
			c = GetCharacter(ctx.fstrType, cur);
		}
		while(c && !IsValidPlaceholder(c)) {
			Placeholder::Option* op = outPlaceholder.GetOption(c);
			if(!op)
				throw syntax_exception();

			outPlaceholder.order[idx++] = (char)c;

			op->Enable();

			AdvanceCursor(ctx.fstrType, cur);
			c = GetCharacter(ctx.fstrType, cur); // Character after placeholder
			if(c == '?') {
				AdvanceCursor(ctx.fstrType, cur);
				op->SetPlaceholder();
				outPlaceholder.placeholderOrder[idx2++] = outPlaceholder.order[idx - 1];
				++subPlaceholderCount;
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

		return cur;
	}

	//! Read until placeholder.
	/**
	Will read until a placeholder is encounterd.
	Will replace ~~ with ~.
	\param ctx The format context.
	\param str The input string.
	\return Pointer to the first character of the placeholder(the one after ~)
	*/
	const char* ParseUntilPlaceholder(
		Context& ctx,
		const char* str,
		Placeholder& outPlaceholder)
	{
		outPlaceholder.type = 0;

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

			// Advance cursor until ~ character.
			uint32_t c;
			const char* before = cur;
			const char* tmp;
			do {
				tmp = cur;
				c = AdvanceCursor(ctx.fstrType, cur);
				++counter;
				++collumns;
				if(c == '\n') {
					++newLines;
					collumns = 0;
				}
			} while(c && c != '~');

			size = tmp - before;

			// The string ended before another placeholder.
			if(c == 0)
				break;

			// Remember offset of the current placeholder
			ctx.fstrLastArgPos = cur - ctx.fstr;

			cur = ParsePlaceholder(ctx, cur, outPlaceholder, outPlaceholder.subPlaceholderCount);

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

		return cur;
	}

	void ReplaceSubPlaceholder(PlaceholderCtx& ctx, int argInt)
	{
		--ctx.ignoreCount;

		char c = ctx.placeholder.placeholderOrder[ctx.subId];
		ctx.subId++;

		if(c == ' ')
			ctx.placeholder.master.SetValue(argInt);
		else
			ctx.placeholder.GetOption(c)->SetValue(argInt);
	}

	void AlignString(Context& ctx, slice* prevSlice, size_t beforeLength, const Placeholder& placeholder)
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
					prevSlice = ctx.InsertSlice(prevSlice, slice(maxCount, SPACES));
					count -= maxCount;
				} else {
					prevSlice = ctx.InsertSlice(prevSlice, slice(count, SPACES));
					return;
				}
			}
		}
	}
}
}
