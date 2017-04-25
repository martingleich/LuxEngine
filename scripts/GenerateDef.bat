
echo LIBRARY LuxEngine > %4
echo EXPORTS >> %4
for %%f in (%2\*.lib) do "%VS140COMNTOOLS%..\..\VC\bin\dumpbin.exe" /LINKERMEMBER:1 "%%f" | "%1" %3 >> %4