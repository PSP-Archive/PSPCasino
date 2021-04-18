@echo off

echo Compiling %1
C:\PS2Dev\gcc\ee\bin\ee-gcc -Wall -march=r4000 -g -mgp32 -mlong32 -c %1
if NOT %errorlevel%==0 ( goto END )

:SUCCESS
echo Successfully compiled %1
:END
