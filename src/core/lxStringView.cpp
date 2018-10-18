#include "core/lxStringView.h"
#include "core/lxUnicode.h"

namespace lux
{
namespace core
{

const StringView StringView::EMPTY;

bool StringView::Equal(const StringView& other, EStringCompare compare) const
{
	if(Size() != other.Size())
		return false;

	switch(compare) {
	case EStringCompare::CaseSensitive:
		return (memcmp(Data(), other.Data(), Size()) == 0);
	case EStringCompare::CaseInsensitive:
	{
		const char* a = Data();
		const char* b = other.Data();
		u32 ac;
		u32 bc;
		do {
			ac = AdvanceCursorUTF8(a);
			bc = AdvanceCursorUTF8(b);

			if(!IsEqualCaseInsensitive(ac, bc))
				return false;
		} while(ac && bc);

		return (ac == bc);
	}
	}
	return false;
}

bool StringView::Smaller(const StringView& other, EStringCompare compare) const
{
	switch(compare) {
	case EStringCompare::CaseSensitive:
	{
		auto size = other.Size();
		auto s = Size() < size ? Size() : size;
		return (memcmp(Data(), other.Data(), s) < 0);
	}
	case EStringCompare::CaseInsensitive:
	{
		const char* a = Data();
		const char* b = other.Data();
		u32 ac;
		u32 bc;
		do {
			ac = AdvanceCursorUTF8(a);
			bc = AdvanceCursorUTF8(b);

			u32 iac = ToLowerChar(ac);
			u32 ibc = ToLowerChar(bc);
			if(iac < ibc)
				return true;
			if(iac > ibc)
				return false;
		} while(ac && bc);

		return (ac < bc);
	}
	}
	return false;
}

bool StringView::StartsWith(const StringView& data, EStringCompare compare) const
{
	if(data.IsEmpty())
		return true;
	if(data.Size() > Size())
		return false;
	return StringView(Data(), data.Size()).Equal(data, compare);
}

bool StringView::EndsWith(const StringView& data, EStringCompare compare) const
{
	if(data.IsEmpty())
		return true;
	if(data.Size() > Size())
		return false;
	return StringView(Data()+Size()-data.Size(), data.Size()).Equal(data, compare);
}

int StringView::Find(const StringView& search) const
{
	if(search.IsEmpty())
		return 0;
	if(search.Size() > Size())
		return -1;

	const char* cur = Data();
	while(cur + search.Size() <= Data() + Size()) {
		if(memcmp(search.Data(), cur, search.Size()) == 0)
			return cur-Data();
		++cur;
	}

	return -1;
}

int StringView::FindReverse(const StringView& search) const
{
	if(search.IsEmpty())
		return Size()-1;
	if(search.Size() > Size())
		return -1;

	const char* cur = Data() + (Size() - search.Size());
	while(cur >= Data()) {
		if(memcmp(search.Data(), cur, search.Size()) == 0)
			return cur-Data();
		--cur;
	}

	return -1;
}

EStringClassFlag StringView::Classify() const
{
	int alphaCount = 0;
	int digitCount = 0;
	int spaceCount = 0;
	int upperCount = 0;
	int lowerCount = 0;
	int count = 0;
	for(auto c : CodePoints()) {
		if(IsLower(c))
			++lowerCount;
		else if(IsUpper(c))
			++upperCount;

		if(IsAlpha(c))
			++alphaCount;
		else if(IsDigit(c))
			++digitCount;
		else if(IsSpace(c))
			++spaceCount;
		++count;
	}

	EStringClassFlag out = (EStringClassFlag)0;
	if(Size() == 0)
		out |= EStringClassFlag::Empty;

	if(lowerCount == alphaCount && alphaCount > 0)
		out |= EStringClassFlag::Lower;
	else if(upperCount == alphaCount && alphaCount > 0)
		out |= EStringClassFlag::Upper;

	if(alphaCount == count && alphaCount > 0)
		out |= EStringClassFlag::Alpha;
	else if(digitCount == count && digitCount > 0)
		out |= EStringClassFlag::Digit;
	else if(alphaCount + digitCount == count && alphaCount > 0 && digitCount > 0)
		out |= EStringClassFlag::AlphaNum;
	else if(spaceCount == count && spaceCount > 0)
		out |= EStringClassFlag::Space;

	return out;
}

bool StringView::IsWhitespace() const
{
	return IsEmpty() || Classify() == EStringClassFlag::Space;
}

}
}
