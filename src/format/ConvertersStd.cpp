#include "format/ConvertersStd.h"

#include "format/ConvInternal.h"
#include "format/UnicodeConversion.h"

namespace format
{
void fmtPrint(Context& ctx, const std::string& data, Placeholder& placeholder)
{
	(void)ctx;

	if(placeholder.type == 'a' || placeholder.type == 's')
		ctx.AddSlice(data.length(), data.c_str());
	else
		throw invalid_placeholder_type("Invalid placeholder for std::string type.", ctx.fstrLastArgPos, ctx.argId, placeholder.type);
}

}
