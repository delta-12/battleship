# Battleship

Simple implementation of the classic Battleship game in C using the SDL library.

## Build Instructions

1. Install C compiler, build system if applicable, and dependencies
2. Clone repository
3. Compile

### Linux

1. Install `gcc`, `make`, and SDL library

   `sudo apt install -y gcc make libsdl2-2.0-0 libsdl2-dev libsdl2-image-2.0-0 libsdl2-image-dev libsdl2-ttf-dev`

2. Clone repository

   `git clone https://github.com/delta-12/battleship.git`

3. Make `bin` and `obj` directories in the cloned repository

   `cd battleship && mkdir bin obj`

4. Compile the game with `make`

   `make`

5. The binary will placed in the `bin` directory and can be run using the following command

   `bin/battleship`

### Mac OS

1. Install `gcc` and `make`
2. Clone repository

   `git clone https://github.com/delta-12/battleship.git`

3. Run the build script

   `cd battleship/macOS && chmod +x build.sh && ./build.sh`

4. The binary will placed in the `bin` directory and can be run using the following command

   `bin/battleship`

### Windows

1. Install a GCC compiler for Windows such as [tdm-gcc](https://jmeubank.github.io/tdm-gcc/download/)
2. Clone repository

   `git clone https://github.com/delta-12/battleship.git`

3. Run the build script

   `cd battleship\windows && build.bat`

4. The binary will placed in the `bin` directory and can be run by double-clicking or running the following command

   `bin\battleship.exe`
