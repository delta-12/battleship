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

# compile game
cd ../../
mkdir ../bin ../obj
gcc -o ../obj/engine.o -c ../src/engine.c -ISDL-install/include -D_THREAD_SAFE
gcc -o ../obj/rendering.o -c ../src/rendering.c -ISDL-install/include -D_THREAD_SAFE
gcc -o ../bin/battleship ../src/battleship.c ../obj/engine.o ../obj/rendering.o -ISDL-install/include -D_THREAD_SAFE -LSDL-install/lib -lSDL2
