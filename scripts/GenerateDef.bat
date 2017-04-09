
echo LIBRARY LuxEngine > %3
echo EXPORTS >> %3
for %%f in (%1\*.lib) do "%VS140COMNTOOLS%..\..\VC\bin\dumpbin.exe" /LINKERMEMBER:1 "%%f" | "%~dp0\..\external\FilterExportFiles\Win32\Release\FilterExportFiles.exe" %2 >> %3