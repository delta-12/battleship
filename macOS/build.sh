#!/bin/zsh

# SDL will be installed to SDL-install directory
mkdir SDL-install

# install SDL
# from SDL installation guide https://wiki.libsdl.org/Installation
git clone https://github.com/libsdl-org/SDL
cd SDL
mkdir build
cd build
../configure --prefix=$(pwd)/../../SDL-install # install SDL to custom directory
make
make install

# Download and extract SDL_ttf
wget https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-devel-2.0.15-mingw.tar.gz -o SDL2_ttf.tar.gz
mkdir SDL2_ttf-tmp
tar xvf SDL2_ttf.tar.gz -C SDL2_ttf-tmp --strip-components=1
rm SDL2_ttf.tar.gz
mv SDL2_ttf-tmp/x86_64-w64-mingw32 SDL2_ttf
rm -Rf SDL2_ttf-tmp
cp ../resources/SDL_ttf.h SDL2_ttf/include/SDL2

# compile game
cd ../../
mkdir ../bin ../obj
cp ../resources/FreeMonoBold.ttf ../bin
gcc -o ../obj/engine.o -c ../src/engine.c -ISDL-install/include -ISDL2_ttf/include -D_THREAD_SAFE
gcc -o ../obj/rendering.o -c ../src/rendering.c -ISDL-install/include -ISDL2_ttf/include -D_THREAD_SAFE
gcc -o ../bin/battleship ../src/battleship.c ../obj/engine.o ../obj/rendering.o -ISDL-install/include -ISDL2_ttf/include -D_THREAD_SAFE -LSDL-install/lib -lSDL2_ttf/lib -lSDL2 -lSDL2_ttf
