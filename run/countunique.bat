@echo off
if "%1" == "" goto error

echo Hashing...
set CYGWIN=nodosfilewarning
for /r %%i in (*.%1) do md5sum %%i >> md5.txt
echo Counting...
for /f "tokens=1" %%i in (md5.txt) do echo %%i >> md5tokens.txt
echo Number of unique files with extension %1 is:
type md5tokens.txt | sort | uniq -c | wc -l
goto end

:error
echo.
echo Usage:
echo  countunique ^<extension^>
echo.
echo Recurses a file hierarchy and counts unique files with the given extension (without the .).
echo.
:endcount