echo off

@REM Download and extract SDL
powershell -Command "Invoke-WebRequest https://www.libsdl.org/release/SDL2-devel-2.0.18-mingw.tar.gz -OutFile SDL2.tar.gz"
mkdir SDL2-tmp
tar xvf SDL2.tar.gz -C SDL2-tmp --strip-components=1
del SDL2.tar.gz
move SDL2-tmp\x86_64-w64-mingw32 SDL2
RMDIR /S /Q SDL2-tmp

@REM Download and extract SDL_ttf
powershell -Command "Invoke-WebRequest https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-devel-2.0.15-mingw.tar.gz -Outfile SDL2_ttf.tar.gz"
mkdir SDL2_ttf-tmp
tar xvf SDL2_ttf.tar.gz -C SDL2_ttf-tmp --strip-components=1
del SDL2_ttf.tar.gz
move SDL2_ttf-tmp\x86_64-w64-mingw32 SDL2_ttf
RMDIR /S /Q SDL2_ttf-tmp
copy ..\resources\SDL_ttf.h SDL2_ttf\include\SDL2

@REM Compile
mkdir ..\bin ..\obj
copy SDL2\bin\SDL2.dll ..\bin
copy SDL2_ttf\bin\*.dll ..\bin
copy ..\resources\FreeMonoBold.ttf ..\bin
gcc -Wall -o ..\obj\engine.o -c ..\src\engine.c -ISDL2\include -ISDL2_ttf\include -D_THREAD_SAFE
gcc -Wall -o ..\obj\rendering.o -c ..\src\rendering.c -ISDL2\include -ISDL2_ttf\include -D_THREAD_SAFE
gcc -Wall -o ..\bin\battleship ..\src\battleship.c ..\obj\engine.o ..\obj\rendering.o -ISDL2\include -ISDL2_ttf\include -D_THREAD_SAFE -DSDL_MAIN_HANDLED -LSDL2\lib -LSDL2_ttf\lib -lSDL2 -lSDL2_ttf -include stdio.h

pause