#include "UnitTestEx.h"

#include <sys/stat.h>

UNIT_SUITE(FileSystem)
{
	StrongRef<LuxDevice> g_Device;
	StrongRef<io::FileSystem> g_FileSys;
	io::path m_WorkingDir;

	void CreateTestDirectory()
	{
#ifdef LUX_WINDOWS
		io::path testDir = m_WorkingDir + "FileSystemTestDir";
		testDir.Replace("\\", "/");
		string cmd = "if not exist \"" + testDir + "\" mkdir \"" + testDir + "\"";
		system(cmd.Data());
#else
		throw "Not implemented";
#endif
	}

	void RemoveTestDirectory()
	{
#ifdef LUX_WINDOWS
		io::path testDir = m_WorkingDir + "FileSystemTestDir";
		testDir.Replace("\\", "/");
		string cmd = "if exist \"" + testDir + "\" rmdir \"" + testDir + "\" /s /q";
		system(cmd.Data());
#else
		throw "Not implemented";
#endif
	}

	bool CheckForFile(const char* p)
	{
		io::path testDir = m_WorkingDir + p;

		struct stat buffer;
		return (stat(testDir.Data(), &buffer) == 0);
	}

	UNIT_SUITE_INIT()
	{
		log::EngineLog.SetLogLevel(log::ELogLevel::None);

		g_Device = lux::CreateDevice();
		g_FileSys = g_Device->GetFileSystem();
		m_WorkingDir = g_FileSys->GetWorkingDirectory();
		
		RemoveTestDirectory();
		CreateTestDirectory();
	}

	UNIT_SUITE_EXIT()
	{
		g_FileSys.Reset();
		g_Device.Reset();

		RemoveTestDirectory();
	}

	UNIT_TEST(CreateFile)
	{
		bool result = g_FileSys->CreateFile("FileSystemTestDir/file");
		UNIT_ASSERT(result == true);
		UNIT_ASSERT(CheckForFile("FileSystemTestDir/file"));
	}

	UNIT_TEST(CreateFile2)
	{
		// This tests creation of directories, too.
		bool result = g_FileSys->CreateFile("FileSystemTestDir/subdir/file", true);
		UNIT_ASSERT(result == true);
		UNIT_ASSERT(CheckForFile("FileSystemTestDir/subdir/file"));
	}
}