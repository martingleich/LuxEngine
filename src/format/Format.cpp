#include "format/Format.h"
#include "format/ConvertersHelper.h"
#include "format/GeneralParsing.h"
#include <climits>
#include <cassert>

namespace format
{
namespace
{
enum class EParseResult
{
	SimplePlaceholder,
	FunctionPlaceholder
};

void ParsePlaceholderOptions(parser::SaveStringReader& reader, Placeholder& outPlaceholder)
{
	if(reader.Peek() == '!') {
		reader.Get();
		auto start = reader.str;
		int brace_level = 0;
		while(true) {
			char c = reader.Get();
			if(c == '{')
				++brace_level;
			if(c == '}')
				--brace_level;

			if(reader.IsEnd())
				break;
			if(reader.Peek() == '}' && brace_level == 0)
				break;
			if(reader.Peek() == ':')
				break;
		}
		outPlaceholder.type = Slice(int(reader.str - start), start);
	} else {
		outPlaceholder.type = Slice();
	}

	if(reader.Peek() == ':') {
		reader.Get();
		auto start = reader.str;
		int brace_level = 0;
		while(true) {
			char c = reader.Get();
			if(c == '{')
				++brace_level;
			if(c == '}')
				--brace_level;

			if(reader.IsEnd())
				break;
			if(reader.Peek() == '}' && brace_level == 0)
				break;
		}
		outPlaceholder.format = Slice(int(reader.str - start), start);
	} else {
		outPlaceholder.format = Slice();
	}
}

// Begins on opening brace
// Ends after closing brace
EParseResult ParsePlaceholder(parser::SaveStringReader& reader, Placeholder& outPlaceholder)
{
	int value;
	SkipSpace(reader);
	if(TryReadInteger(reader, value))
		outPlaceholder.argId = value;
	else if(reader.Peek() == '}' || reader.Peek() == ':' || reader.Peek() == '!')
		outPlaceholder.argId = -1;
	else
		return EParseResult::FunctionPlaceholder;

	SkipSpace(reader);
	ParsePlaceholderOptions(reader, outPlaceholder);
	SkipSpace(reader);

	if(!reader.CheckAndGet('}'))
		throw format_exception("Missing closing brace for placeholder");

	return EParseResult::SimplePlaceholder;
}

bool ParseUntilPlaceholder(Context& ctx, parser::SaveStringReader& reader)
{
	auto before = reader.str;
START:
	// Advance cursor until brace character.
	while(!reader.IsEnd() && reader.Peek() != '{')
		reader.Get();
	// Flush output
	if(reader.str != before)
		ctx.AddSlice(int(reader.str - before), before);
	// The string ended before another placeholder.
	if(reader.IsEnd())
		return false;
	// Move to first character of placeholder
	reader.Get();

	// Double brace escape -> Try again
	if(!reader.IsEnd() && reader.Peek() == '{') {
		before = reader.str;
		reader.Get();
		goto START;
	}

	ctx.SetCurPlaceholderOffset(int(reader.str - reader.begin));

	return true;
}

void WriteSimplePlaceholder(Context& ctx, Placeholder& pl)
{
	auto entry = ctx.GetFormatEntry(pl.argId);
	entry->Convert(ctx, pl);
}

class PlaceholderId
{
public:
	PlaceholderId(Context& _ctx) :
		ctx(_ctx),
		value(-1)
	{
	}
	int RetrieveNextValue(int arg)
	{
		value = arg + 1;
		ctx.SetCurArgId(arg);
		return arg;
	}
	int RetrieveNextValue()
	{
		++value;
		ctx.SetCurArgId(value);
		return value;
	}

private:
	Context& ctx;
	int value;
};

class FunctionParser
{
private:
	Context& ctx;
	parser::SaveStringReader& reader;
	PlaceholderId& id;

public:
	FunctionParser(Context& _ctx, parser::SaveStringReader& _reader, PlaceholderId& _id) :
		ctx(_ctx),
		reader(_reader),
		id(_id)
	{
	}

	int ParsePlaceholderValue()
	{
		format_exception error("invalid_format_function_call: value");
		if(!reader.CheckAndGet('{'))
			throw error;
		int value;
		if(parser::TryReadInteger(reader, value))
			value = id.RetrieveNextValue(value);
		else
			value = id.RetrieveNextValue();
		return value;
	}

	Facet_Functions::Value ParseValue(Facet_Functions::EValueKind kind)
	{
		Facet_Functions::Value out;
		format_exception error("invalid_format_function_call: value");
		if(kind == Facet_Functions::EValueKind::Integer) {
			int value;
			if(reader.Peek() == '{') {
				// Placeholder entry.
				value = ParsePlaceholderValue();
				if(!reader.CheckAndGet('}'))
					throw error;
				value = ctx.GetFormatEntry(value)->AsInteger();
				out.iValue = value;
			} else {
				// Literal value.
				if(!parser::TryReadInteger(reader, value))
					throw error;
			}
		} else if(kind == Facet_Functions::EValueKind::Placeholder) {
			int value = ParsePlaceholderValue();
			out.pValue = ctx.GetFormatEntry(value);
		} else if(kind == Facet_Functions::EValueKind::String) {
			if(reader.Peek() == '{') {
				// Placeholder entry.
				int value = ParsePlaceholderValue();
				SkipSpace(reader);
				Placeholder pl;
				ParsePlaceholderOptions(reader, pl);
				if(!reader.CheckAndGet('}'))
					throw error;
				auto entry = ctx.GetFormatEntry(value);
				Slice result;
				{
					Context::OutputStateContext osc(ctx);
					entry->Convert(ctx, pl);
					result = osc.CreateSlice();
				}
				out.sValue = result;
			} else if(reader.Peek() == '\'') {
				reader.Get();
				auto start = reader.str;
				while(!reader.IsEnd() && reader.Peek() != '\'')
					reader.Get();
				if(reader.IsEnd())
					throw error;
				int length = int(reader.str - start);
				out.sValue = Slice(length, start);
				if(!reader.CheckAndGet('\''))
					throw error;
			} else {
				throw error;
			}
		} else {
			throw error;
		}
		return out;
	}

	void WriteAndParseFunctionPlaceholder()
	{
		/*
		Some functions must be implemented in here for example if.
		A few basic operators would be nice as well(
			comparision for int(relational and equality) and string(only equality), result is integer(0 or 1).
			for integers some operators(add sub mul div negate) and brackets.
			The parser must be pretty damn fast(test recursive vs shunting yard)
			Allow inner functions calls.
			All kinds of returns should be handled.
		*/
		format_exception error("invalid_format_function_call");
		parser::SkipSpace(reader);
		auto func_name = parser::ReadWord(reader);
		int argCount;
		const Facet_Functions::EValueKind* argTypes;
		Facet_Functions::EValueKind retType;
		auto& functions = ctx.GetLocale()->GetFunctionsFacet();
		auto func = functions.GetFunction(func_name, argCount, argTypes, retType);
		if(!func)
			throw format_exception("Unknown function");
		if(!reader.CheckAndGet('('))
			throw format_exception("Missing opening parenthesis from function call");

		Facet_Functions::Value args[10];
		int arg_id = 0;
		while(arg_id < argCount) {
			if(arg_id >= 10)
				throw format_exception("Only 10 functions arguments allowed");
			parser::SkipSpace(reader);
			args[arg_id] = ParseValue(argTypes[arg_id]);
			++arg_id;
			parser::SkipSpace(reader);
			if(arg_id != argCount) {
				if(!reader.CheckAndGet(','))
					throw format_exception("Missing comma between function arguments");
			}
		}

		parser::SkipSpace(reader);

		if(!reader.CheckAndGet(')'))
			throw format_exception("Missing closing parenthesis from function call");

		Placeholder pl;
		ParsePlaceholderOptions(reader, pl);
		pl.argId = -1;

		if(!reader.CheckAndGet('}'))
			throw error;

		auto retValue = func(ctx, args);
		if(retType == Facet_Functions::EValueKind::String)
			ctx.AddSlice(retValue.sValue);
		else if(retType == Facet_Functions::EValueKind::Integer)
			throw format_exception("Can't handle function return type."); // TODO: How to print the integer???
		else if(retType == Facet_Functions::EValueKind::Placeholder)
			retValue.pValue->Convert(ctx, pl);
		else
			throw format_exception("Can't handle function return type.");
	}
};
} // end anynmous namespace

namespace internal
{
void format(Context& ctx, Slice fmtStr)
{
	Context::SubContext subCtx(ctx);
	parser::SaveStringReader reader(fmtStr);

	PlaceholderId plId(ctx);
	Placeholder pl;

	while(ParseUntilPlaceholder(ctx, reader)) {
		auto result = ParsePlaceholder(reader, pl);

#if defined(FORMAT_ERROR_TEXT) && defined(FORMAT_NO_EXCEPTIONS)
		try {
#endif
			if(result == EParseResult::SimplePlaceholder) {
				int id;
				if(pl.argId >= 0)
					id = plId.RetrieveNextValue(pl.argId);
				else
					id = plId.RetrieveNextValue();
				pl.argId = id;
				WriteSimplePlaceholder(ctx, pl);
			} else if(result == EParseResult::FunctionPlaceholder) {
				FunctionParser parser(ctx, reader, plId);
				parser.WriteAndParseFunctionPlaceholder();
			} else {
				throw format_exception("invalid_placeholder_type");
			}
#if defined(FORMAT_ERROR_TEXT) && defined(FORMAT_NO_EXCEPTIONS)
		} catch(const format_exception& exp) {
			ctx.AddTerminatedSlice("<FORMAT_ERROR:");
			ctx.AddTerminatedSlice(exp.msg);
			ctx.AddTerminatedSlice(">");
		}
#endif
	}
}

}
}
