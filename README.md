# Battleship

Simple implementation of the classic Battleship game in C using the SDL library.

# Build Instruction

1. Install C compiler, build system if applicable, and dependencies
2. Clone repository
3. Compile

## Linux

1. Install `gcc`, `make`, and SDL library

`sudo apt install gcc make libsdl2-2.0-0 libsdl2-dev libsdl2-image-2.0-0 libsdl2-image-dbg libsdl2-image-dev -y`

2. Clone repository

`git clone https://github.com/delta-12/battleship.git`

3. Make `bin` and `obj` directories in the cloned repository

`cd battleship && mkdir bin obj`

4. Compile the game with `make`

`make`

5. The binary will placed in the `bin` directory and can be ran using the following command

`bin/battleship`
