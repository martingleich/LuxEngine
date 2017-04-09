@echo off

if not exist "%VS140COMNTOOLS%..\..\VC\bin\dumpbin.exe" (
	echo Missing dumpbin.exe, Visual Studio 2015 is missing or corrupted.
	echo Search path is: "%VS140COMNTOOLS%..\..\VC\bin\dumpbin.exe"
	exit /b 1
)