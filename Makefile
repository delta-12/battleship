.PHONY = all clean

CC = gcc
CFLAGS = `sdl2-config --libs --cflags` -ggdb3 -O0 --std=c99 -Wall -lSDL2_image -lm
SRC = src
OBJ = obj
BIN = bin
SRCS = $(wildcard $(SRC)/*.c)
OBJS = $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))
BINS = $(patsubst $(SRC)/%.c, $(BIN)/%, $(SRCS))

all: engine.o rendering.o battleship

engine.o: $(SRC)/engine.c $(SRC)/engine.h
	${CC} ${CFLAGS} -o $(OBJ)/engine.o -c $(SRC)/engine.c

rendering.o: $(SRC)/rendering.c $(SRC)/rendering.h
	${CC} ${CFLAGS} -o $(OBJ)/rendering.o -c $(SRC)/rendering.c

battleship: $(SRC)/battleship.c $(OBJ)/engine.o $(OBJ)/rendering.o
	${CC} ${CFLAGS} -o $(BIN)/$@ $^
	# $(BIN)/$@

clean:
	@echo "Cleaning up..."
	@rm -rvf ${BINS} ${OBJS} $(SRC)/*.o