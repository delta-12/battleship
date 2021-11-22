.PHONY = all clean

CC = gcc
CFLAGS = `sdl2-config --libs --cflags` -ggdb3 -O0 --std=c99 -Wall -lSDL2_image -lm
SRC = src
OBJ = obj
BIN = bin
SRCS = $(wildcard $(SRC)/*.c)
OBJS = $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))
BINS = $(patsubst $(SRC)/%.c, $(BIN)/%, $(SRCS))

all: engine.o battleship.o main

engine.o: $(SRC)/engine.c $(SRC)/engine.h
	${CC} ${CFLAGS} -o $(OBJ)/engine.o -c $(SRC)/engine.c

battleship: $(SRC)/battleship.c $(OBJ)/engine.o
	${CC} ${CFLAGS} -o $(BIN)/$@ $^
	# $(BIN)/$@

# %.o: $(SRC)/%.c $(SRC)/%.h
# 	$(CC) $(CFLAGS) -o $(OBJ)/$@ -c $^

main: $(SRC)/main.c $(OBJ)/ansi_escapes.o $(OBJ)/core.o $(OBJ)/hanoi.o $(SRC)/main_resources.h
	${CC} ${CFLAGS} -o $(BIN)/$@ $^
	$(BIN)/$@

test: $(SRC)/test.c
	${CC} ${CFLAGS}-o $(BIN)/$@ $<
	# $(BIN)/$@

clean:
	@echo "Cleaning up..."
	@rm -rvf ${BINS} ${OBJS} $(SRC)/*.o