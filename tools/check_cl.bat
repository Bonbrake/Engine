@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1
where cl > tools\cl_where.txt 2>&1
cl > tools\cl_out.txt 2>&1
echo EXIT=%ERRORLEVEL% >> tools\cl_out.txt
