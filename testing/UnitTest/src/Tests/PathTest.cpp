#include "stdafx.h"

UNIT_SUITE(path)
{
	UNIT_TEST(GetFileDir)
	{
		io::Path p = "folder1/folder2\\file.jpg";
		io::Path dir = p.GetFileDir();
		UNIT_ASSERT_STR(dir, "folder1/folder2");
	}

	UNIT_TEST(GetFileExtension)
	{
		io::Path p = "folder1/file.jpg";
		core::String ext = p.GetFileExtension();
		UNIT_ASSERT_STR(ext, "jpg");
	}

	UNIT_TEST(GetFileName1)
	{
		io::Path p = "folder1/file.jpg";
		core::String name = p.GetFileName();
		UNIT_ASSERT_STR(name, "file.jpg");
	}

	UNIT_TEST(GetFileName2)
	{
		io::Path p = "folder1/file.jpg";
		core::String name = p.GetFileName(false);
		UNIT_ASSERT_STR(name, "file");
	}

	UNIT_TEST(GetFileName3)
	{
		io::Path p = "file.jpg";
		core::String name = p.GetFileName(false);
		UNIT_ASSERT_STR(name, "file");
	}

	UNIT_TEST(MakeAbsolutePath)
	{
		io::Path abs = "C:/Folder1/Folder2";
		io::Path rel = "../file";

		UNIT_ASSERT_STR(abs.GetResolved(abs), "C:/Folder1/Folder2");
		UNIT_ASSERT_STR(rel.GetResolved(abs), "C:/Folder1/file");
	}
}