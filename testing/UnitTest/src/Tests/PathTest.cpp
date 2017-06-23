#include "stdafx.h"

UNIT_SUITE(path)
{
	UNIT_TEST(GetFileDir)
	{
		io::path p = "folder1/folder2\\file.jpg";
		io::path dir = io::GetFileDir(p);
		UNIT_ASSERT_STR(dir, "folder1/folder2/");
	}

	UNIT_TEST(GetFileExtension)
	{
		io::path p = "folder1/file.jpg";
		string ext = io::GetFileExtension(p);
		UNIT_ASSERT_STR(ext, "jpg");
	}

	UNIT_TEST(GetFileName1)
	{
		io::path p = "folder1/file.jpg";
		string name = io::GetFilenameOnly(p);
		UNIT_ASSERT_STR(name, "file.jpg");
	}

	UNIT_TEST(GetFileName2)
	{
		io::path p = "folder1/file.jpg";
		string name = io::GetFilenameOnly(p, false);
		UNIT_ASSERT_STR(name, "file");
	}

	UNIT_TEST(GetFileName3)
	{
		io::path p = "file.jpg";
		string name = io::GetFilenameOnly(p, false);
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
		io::path abs = "C:/Folder1/Folder2";
		io::path rel = "../file";

		UNIT_ASSERT_STR(io::MakeAbsolutePath(abs, abs), "C:/Folder1/Folder2");
		UNIT_ASSERT_STR(io::MakeAbsolutePath(abs, rel), "C:/Folder1/file");
	}
}