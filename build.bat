@echo off

mkdir .\build
pushd .\build
ml64 /Zi /c /Cp /Fl /Fo ..\code\calculate_mandelbrot.obj ..\code\calculate_mandelbrot.asm && cl -Zi ..\code\main.cpp user32.lib Gdi32.lib ..\code\calculate_mandelbrot.obj
popd

:: Don't forget /arch:AVX
