#ifndef INCLUDED_UNIT_TEST_EX_H
#define INCLUDED_UNIT_TEST_EX_H
#include "UnitTest.h"
#include "Lux.h"
#include <sstream>
#include <algorithm>
#include <iterator>	


using namespace lux;

template <typename T>
class StringConstants
{
public:
};

template <>
class StringConstants<char>
{
public:
	static char DoubleQuotes() { return '\"'; }
	static char OpenBracket() { return '('; }
	static char CloseBracket() { return ')'; }
	static char Space() { return ' '; }
	static char Equal() { return '='; }
};

template <typename T>
bool IsStringConstant(const T* s)
{
	if(*s == StringConstants<T>::DoubleQuotes())
		return true;
	else
		return false;
}

template <typename T>
std::string MakeWideString(std::basic_string<T> str)
{
	std::string out;
	std::transform(str.begin(), str.end(), std::inserter(out, out.begin()), [](T c)->char { return (char)c; });

	return out;
}

template <typename T>
void AssertString(const T* str1, const T* str2, const char* name1, const char* name2, int line, UnitTesting::TestContext& ctx)
{
	std::basic_stringstream<T> ss;
	if(IsStringConstant(name1))
		ss << name1;
	else
		ss << name1 << StringConstants<T>::OpenBracket() << (str1 ? str1 : "") << StringConstants<T>::CloseBracket();

	ss << StringConstants<T>::Space() << StringConstants<T>::Equal() << StringConstants<T>::Space();

	if(IsStringConstant(name2))
		ss << name2;
	else
		ss << name2 << StringConstants<T>::OpenBracket() << (str2 ? str2 : "") << StringConstants<T>::CloseBracket();

	bool result;
	if(str1 && str2) {
		const T* c1 = str1;
		const T* c2 = str2;
		while(*c1 == *c2 && *c1 && *c2) {
			++c1;
			++c2;
		}

		result = (*c1 == *c2);
	} else {
		result = (str1 == str2);
	}

	ctx.AddResult(UnitTesting::Info("", UnitTesting::USE_PARENT_FILE, line), result, MakeWideString(ss.str()));
}

#define expand_Stringify(s) Stringify(s)
#define Stringify(s) #s

#define UNIT_ASSERT_CSTR(cstr1, cstr2) do{AssertString(cstr1, cstr2, expand_Stringify(cstr1), expand_Stringify(cstr2), __LINE__, ctx);}while(false)
#define UNIT_ASSERT_STR(str1, cstr2) do{AssertString(str1.Data(), cstr2, expand_Stringify(str1), expand_Stringify(cstr2), __LINE__, ctx);}while(false)

#define UNIT_ASSERT_APPROX(a, b) UNIT_ASSERT(math::IsEqual(a, b))

#endif // #ifndef INCLUDED_UNIT_TEST_EX_H