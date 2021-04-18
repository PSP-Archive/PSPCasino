@echo off

call Compile SOLID.c
if NOT %errorlevel%==0 ( goto END )

call Compile Graphics.c
if NOT %errorlevel%==0 ( goto END )

call Compile Controller.c
if NOT %errorlevel%==0 ( goto END )

c:\ps2dev\gcc\ee\bin\ee-gcc -Wall -march=r4000 -g -mgp32 -c -xassembler -O -o startup.o startup.s
if NOT %errorlevel%==0 ( goto END )

echo Linking...
C:\PS2Dev\gcc\ee\bin\ee-ld startup.o SOLID.o Graphics.o Controller.o -M -Ttext 8900000 -q -o out > SOLID.map
if NOT %errorlevel%==0 ( goto END )
echo Linked!

outpatch
f:\elf2pbp\elf2pbp outp "BlackJack v0.2"

:END
