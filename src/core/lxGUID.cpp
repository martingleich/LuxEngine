#include "core/lxGUID.h"

namespace lux
{
namespace core
{

GUID GUID::EMPTY = GUID();

static char NibbleToHex(u8 nibble)
{
	return (nibble < 10 ? '0' : 'A') + nibble;
}

void fmtPrint(format::Context& ctx, const core::GUID& g, format::Placeholder& placeholder)
{
	LUX_UNUSED(placeholder);
	char buffer[36];
	char* cur = buffer;
	for(int i = 0; i < 16; ++i) {
		*cur++ = NibbleToHex((g.Bytes()[i] & 0xF0) >> 8);
		*cur++ = NibbleToHex((g.Bytes()[i] & 0x0F) >> 0);
		if(i == 4 || i == 6 || i == 8 || i == 10)
			*cur++ = '-';
	}
	ctx.AddSlice(36, buffer, true);
}

} // namespace core
} // namespace lux