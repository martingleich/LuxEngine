REM 1: MSCV_PATH 2: FilterExportFiles 3: library dir 4: export.txt 5: output file

echo LIBRARY LuxEngine > "%~5"
echo EXPORTS >> "%~5"

if exist "%~1\dumpbin.exe" (
	set DUMP_BIN="%~1\dumpbin.exe" /LINKERMEMBER:1
) else if exist "%~1\link.exe" (
	set DUMP_BIN="%~1\link.exe" /DUMP /LINKERMEMBER:1
) else if exist "dumpbin.exe" (
	set DUMP_BIN=dumpbin.exe /LINKERMEMBER:1
) else (
	echo Missing dumpbin, can't create def files.
	exit /b 1
)

for %%f in (%~3\*.lib) do %DUMP_BIN% "%%f" | "%~2" "%~4" >> "%~5"