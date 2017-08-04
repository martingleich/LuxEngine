#include "format/ConvertersStd.h"

#include "format/ConvInternal.h"
#include "format/UnicodeConversion.h"

namespace format
{
void conv_data(Context& ctx, const std::string& data, Placeholder& placeholder)
{
	(void)ctx;

	if(placeholder.type == 'a' || placeholder.type == 's') {
		ConvertAddString(ctx, StringType::FORMAT_STRING_TYPE, data.c_str(), data.length());
	} else {
		throw invalid_placeholder_type("Invalid placeholder for std::string type.", ctx.fstrLastArgPos, ctx.argId, placeholder.type);
	}
}

}
