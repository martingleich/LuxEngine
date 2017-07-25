#include "stdafx.h"

UNIT_SUITE(path)
{
	UNIT_TEST(GetFileDir)
	{
		io::Path p = "folder1/folder2\\file.jpg";
		io::Path dir = io::GetFileDir(p);
		UNIT_ASSERT_STR(dir, "folder1/folder2/");
	}

	UNIT_TEST(GetFileExtension)
	{
		io::Path p = "folder1/file.jpg";
		String ext = io::GetFileExtension(p);
		UNIT_ASSERT_STR(ext, "jpg");
	}

	UNIT_TEST(GetFileName1)
	{
		io::Path p = "folder1/file.jpg";
		String name = io::GetFilenameOnly(p);
		UNIT_ASSERT_STR(name, "file.jpg");
	}

	UNIT_TEST(GetFileName2)
	{
		io::Path p = "folder1/file.jpg";
		String name = io::GetFilenameOnly(p, false);
		UNIT_ASSERT_STR(name, "file");
	}

	UNIT_TEST(GetFileName3)
	{
		io::Path p = "file.jpg";
		String name = io::GetFilenameOnly(p, false);
		UNIT_ASSERT_STR(name, "file");
	}

	UNIT_TEST(ConcatFileDesc)
	{
		io::FileDescription desc;
		desc.SetPath("folder1/folder2");
		desc.SetName("file.txt");
		io::FileDescription desc2 = io::ConcatFileDesc(desc, "folder3\\file2.txt");
		UNIT_ASSERT_STR(desc2.GetPath(), "folder1/folder2/folder3/");
		UNIT_ASSERT_STR(desc2.GetName(), "file2.txt");
	}

	UNIT_TEST(MakeAbsolutePath)
	{
		io::Path abs = "C:/Folder1/Folder2";
		io::Path rel = "../file";

		UNIT_ASSERT_STR(io::MakeAbsolutePath(abs, abs), "C:/Folder1/Folder2");
		UNIT_ASSERT_STR(io::MakeAbsolutePath(abs, rel), "C:/Folder1/file");
	}
}