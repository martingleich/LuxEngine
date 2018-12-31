#include "format/ConvertersStd.h"

#include "format/ConvertersHelper.h"

namespace format
{
void fmtPrint(Context& ctx, const std::string& data, const Placeholder&)
{
	ctx.AddSlice(int(data.length()), data.c_str());
}

}
