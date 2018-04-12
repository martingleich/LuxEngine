#include "stdafx.h"

UNIT_SUITE(String)
{
	UNIT_SUITE_DEPEND_ON(unicode);

	UNIT_TEST(empty_String)
	{
		core::String str;
		UNIT_ASSERT_EQUAL(str.Size(), 0);
		UNIT_ASSERT_EQUAL(str.First(), str.End());
	}

	UNIT_TEST(from_cstr)
	{
		core::String str("Tes☠t");
		UNIT_ASSERT_EQUAL(core::IteratorDistance(str.First(), str.End()), 5);
		UNIT_ASSERT_EQUAL(str.Size(), 7);
		u32 chars[5] = {'T', 'e', 's', 0x2620, 't'};
		int i = 0;
		for(auto it = str.First(); it != str.End(); ++it, ++i)
			UNIT_ASSERT_EQUAL(chars[i], *it);
	}

	UNIT_TEST(from_cstr_partial)
	{
		core::String str("Tes☠ta☠", 5);
		UNIT_ASSERT_EQUAL(core::IteratorDistance(str.First(), str.End()), 5);
		UNIT_ASSERT_EQUAL(str.Size(), 7);
		u32 chars[5] = {'T', 'e', 's', 0x2620, 't'};
		int i = 0;
		for(auto it = str.First(); it != str.End(); ++it, ++i)
			UNIT_ASSERT_EQUAL(chars[i], *it);
	}

	UNIT_TEST(copy_String)
	{
		core::String base("Tes☠t");
		core::String str(base);

		UNIT_ASSERT_EQUAL(core::IteratorDistance(str.First(), str.End()), 5);
		UNIT_ASSERT_EQUAL(str.Size(), 7);
		u32 chars[5] = {'T', 'e', 's', 0x2620, 't'};
		int i = 0;
		for(auto it = str.First(); it != str.End(); ++it, ++i)
			UNIT_ASSERT_EQUAL(chars[i], *it);
	}

	UNIT_TEST(short_String)
	{
		core::String str("ab");
		UNIT_ASSERT_EQUAL(core::IteratorDistance(str.First(), str.End()), 2);
		UNIT_ASSERT_EQUAL(str.Size(), 2);

		u32 chars[5] = {'a', 'b'};
		int i = 0;
		for(auto it = str.First(); it != str.End(); ++it, ++i)
			UNIT_ASSERT_EQUAL(chars[i], *it);
	}
	UNIT_TEST(long_String)
	{
		const char* chars ="abcdefghijklmnopqr";
		core::String str(chars);
		UNIT_ASSERT_EQUAL(core::IteratorDistance(str.First(), str.End()), 18);
		UNIT_ASSERT_EQUAL(str.Size(), 18);

		int i = 0;
		for(auto it = str.First(); it != str.End(); ++it, ++i)
			UNIT_ASSERT_EQUAL((u32)chars[i], *it);
	}

	UNIT_TEST(assign_String)
	{
		core::String str = "abc";
		str = "Tes☠t";

		UNIT_ASSERT_EQUAL(core::IteratorDistance(str.First(), str.End()), 5);
		UNIT_ASSERT_EQUAL(str.Size(), 7);
		u32 chars[5] = {'T', 'e', 's', 0x2620, 't'};
		int i = 0;
		for(auto it = str.First(); it != str.End(); ++it, ++i)
			UNIT_ASSERT_EQUAL(chars[i], *it);
	}

	UNIT_TEST(compare_String)
	{
		core::String a = "abc";
		core::String b = "Tes☠t";
		core::String c = "Tes☠t";
		core::String d = "Tes☠a";
		core::String e = "";

		UNIT_ASSERT_TRUE(a != b);
		UNIT_ASSERT_TRUE(b == c);
		UNIT_ASSERT_TRUE(a != e);
		UNIT_ASSERT_TRUE(b != d);
		UNIT_ASSERT_TRUE(a == a);
	}

	UNIT_TEST(compare_case_insensitive)
	{
		core::String a = "abc";
		core::String b = "Tes☠t";
		core::String d = "tes☠T";
		core::String e = "";

		UNIT_ASSERT_TRUE(a.Equal(b, core::EStringCompare::CaseInsensitive) == false);
		UNIT_ASSERT_TRUE(b.Equal(d, core::EStringCompare::CaseInsensitive) == true);
		UNIT_ASSERT_TRUE(b.Equal(e, core::EStringCompare::CaseInsensitive) == false);
	}

	UNIT_TEST(sub_String_1)
	{
		core::String a = "Tes☠a";

		UNIT_ASSERT_EQUAL(a.SubString(a.First() + 2, a.End()), "s☠a");
		UNIT_ASSERT_EQUAL(a.SubString(a.First(), a.First()), "");
	}

	UNIT_TEST(sub_String_2)
	{
		core::String a = "Tes☠a";

		UNIT_ASSERT_EQUAL(a.SubString(a.First() + 2, 2), "s☠");
		UNIT_ASSERT_EQUAL(a.SubString(a.First(), 0), "");
	}

	UNIT_TEST(insert_middle)
	{
		core::String a = "FooBar";

		a.Insert(a.First() + 3, "Bib");

		UNIT_ASSERT_EQUAL(a, "FooBibBar");
	}

	UNIT_TEST(insert_begin)
	{
		core::String a = "FooBar";

		a.Insert(a.First(), "Bib");

		UNIT_ASSERT_EQUAL(a, "BibFooBar");
	}

	UNIT_TEST(insert_end)
	{
		core::String a = "FooBar";

		a.Insert(a.End(), "Bib");

		UNIT_ASSERT_EQUAL(a, "FooBarBib");
	}

	UNIT_TEST(insert_empty)
	{
		core::String a = "str";

		a.Insert(a.First(), "");
		UNIT_ASSERT_EQUAL(a, "str");
		a = "";
		a.Insert(a.First(), "str");
		UNIT_ASSERT_EQUAL(a, "str");
	}

	UNIT_TEST(resize)
	{
		core::String a = "abc";
		a.Resize(1);
		UNIT_ASSERT_EQUAL(a, "a");

		a = "abc";
		a.Resize(0);
		UNIT_ASSERT_EQUAL(a, "");

		a = "abc";
		a.Resize(3);
		UNIT_ASSERT_EQUAL(a, "abc");

		a = "abc";
		a.Resize(5);
		UNIT_ASSERT_EQUAL(a, "abc  ");

		a = "abc";
		a.Resize(8, "12");
		UNIT_ASSERT_EQUAL(a, "abc12121");
	}

	UNIT_TEST(start_width)
	{
		core::String str = "FooBarBib";
		core::String e = "";

		UNIT_ASSERT_TRUE(str.StartsWith(""));
		UNIT_ASSERT_TRUE(str.StartsWith("Foo"));
		UNIT_ASSERT_FALSE(str.StartsWith("Blubdsafklhaasdjlfk"));
		UNIT_ASSERT_TRUE(str.StartsWith("Bar", str.First() + 3));
		UNIT_ASSERT_FALSE(e.StartsWith("Bar"));
	}

	UNIT_TEST(end_width)
	{
		core::String str = "FooBarBib";
		core::String e = "";

		UNIT_ASSERT_TRUE(str.EndsWith(""));
		UNIT_ASSERT_TRUE(str.EndsWith("Bib"));
		UNIT_ASSERT_FALSE(str.EndsWith("Blubdsafklhaasdjlfk"));
		UNIT_ASSERT_TRUE(str.EndsWith("Foo", str.First() + 3));
		UNIT_ASSERT_FALSE(e.EndsWith("Bar"));
	}

	UNIT_TEST(find)
	{
		core::String str = "FooBarBib";

		UNIT_ASSERT_EQUAL(str.Find("Bar"), str.First() + 3);
		UNIT_ASSERT_EQUAL(str.Find("Bib"), str.First() + 6);
		UNIT_ASSERT_EQUAL(str.Find("Blub"), str.End());
		UNIT_ASSERT_EQUAL(str.Find(""), str.End());
	}

	UNIT_TEST(find_reverse)
	{
		core::String str = "FooBarBib";

		UNIT_ASSERT_EQUAL(str.FindReverse("Bar"), str.First() + 3);
		UNIT_ASSERT_EQUAL(str.FindReverse("Bib"), str.First() + 6);
		UNIT_ASSERT_EQUAL(str.FindReverse("Blub"), str.End());
		UNIT_ASSERT_EQUAL(str.FindReverse(""), str.End());
	}

	UNIT_TEST(replace_range)
	{
		core::String str = "Hallo Welt";
		str.ReplaceRange("Bye", str.First(), 5);
		UNIT_ASSERT_EQUAL(str, "Bye Welt");
		str = "Hallo Welt";
		str.ReplaceRange(" du schöne ", str.First()+5, 1);
		UNIT_ASSERT_EQUAL(str, "Hallo du schöne Welt");
		str = "Hallo Welt";
		str.ReplaceRange("", str.First()+5, 5);
		UNIT_ASSERT_EQUAL(str, "Hallo");
	}

	UNIT_TEST(rstrip)
	{
		core::String str = "Hallo";
		UNIT_ASSERT_EQUAL(str.RStrip(), "Hallo");
		str = "";
		UNIT_ASSERT_EQUAL(str.RStrip(), "");
		str = "Hallo  \t\n";
		UNIT_ASSERT_EQUAL(str.RStrip(), "Hallo");
		str = "\t\t\n ";
		UNIT_ASSERT_EQUAL(str.RStrip(), "");
	}

	UNIT_TEST(lstrip)
	{
		core::String str = "Hallo";
		UNIT_ASSERT_EQUAL(str.LStrip(), "Hallo");
		str = "";
		UNIT_ASSERT_EQUAL(str.LStrip(), "");
		str = "\t\n  Hallo";
		UNIT_ASSERT_EQUAL(str.LStrip(), "Hallo");
		str = "\t\t\n ";
		UNIT_ASSERT_EQUAL(str.LStrip(), "");
	}

	UNIT_TEST(pop)
	{
		core::String str = "abc";
		UNIT_ASSERT_EQUAL(str.Pop(1), 1);
		UNIT_ASSERT_EQUAL(str, "ab");
		str = "ab☠c";
		UNIT_ASSERT_EQUAL(str.Pop(2), 2);
		UNIT_ASSERT_EQUAL(str, "ab");
		str = "abc";
		UNIT_ASSERT_EQUAL(str.Pop(100), 3);
		UNIT_ASSERT_EQUAL(str, "");
	}

	UNIT_TEST(split)
	{
		core::String str = "ab,b,,c";
		core::String out[5];
		size_t count = str.Split(',', out, 5);
		UNIT_ASSERT_EQUAL(count, 4);
		UNIT_ASSERT_EQUAL(out[0], "ab");
		UNIT_ASSERT_EQUAL(out[1], "b");
		UNIT_ASSERT_EQUAL(out[2], "");
		UNIT_ASSERT_EQUAL(out[3], "c");
	}
}