
// initialize window
// draw grid
// matrix 0 or 1
// place pieces
// after a hit, check pointer array for ship sank

#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "engine.h"

#define BOARD_SIZE 17
#define CELL_SIZE 36
#define FPS 60

// general ship struct
typedef struct
{
    int *pos[5];
    int len, rot; // rotation- 0 left, 1 up, 2 right, 3 down
    bool isPlaced;
} ship;

typedef struct
{
    // player's boards
    int playerGrid[BOARD_SIZE][BOARD_SIZE];
    int opponentGrid[BOARD_SIZE - 7][BOARD_SIZE - 7];

    // player's ships
    ship carrier;
    ship cruiser;
    ship destroyer;
    ship submarine;
    ship patrolBoat;
} player;

void initializeBoards(player *p)
{
    // zero out grids
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            p->playerGrid[i][j] = 0;
            if (i < BOARD_SIZE - 7 && j < BOARD_SIZE - 7)
                p->opponentGrid[i][j] = 0;
        }
    }
}

void initializeShips(player *p)
{
    // initial position pointer arrays
    p->carrier.len = 5;
    p->cruiser.len = 4;
    p->destroyer.len = 3;
    p->submarine.len = 3;
    p->patrolBoat.len = 2;

    // initial orientation up
    p->carrier.rot = 0;
    p->cruiser.rot = 0;
    p->destroyer.rot = 0;
    p->submarine.rot = 0;
    p->patrolBoat.rot = 0;

    // ships not placed on board yet
    p->carrier.isPlaced = false;
    p->cruiser.isPlaced = false;
    p->destroyer.isPlaced = false;
    p->submarine.isPlaced = false;
    p->patrolBoat.isPlaced = false;

    // place ships to right of player board
    // p->carrier.pos[0] = &p->playerGrid[11][1];
    // p->cruiser.pos[0] = &p->playerGrid[11][3];
    // p->destroyer.pos[0] = &p->playerGrid[11][5];
    // p->submarine.pos[0] = &p->playerGrid[11][7];
    // p->patrolBoat.pos[0] = &p->playerGrid[11][9];

    // placeShip(&p->carrier);
    // placeShip(&p->cruiser);
    // placeShip(&p->destroyer);
    // placeShip(&p->submarine);
    // placeShip(&p->patrolBoat);

    for (int i = 0; i < p->carrier.len; i++)
    {
        p->playerGrid[11 + i][1] = 1;
        p->carrier.pos[i] = &p->playerGrid[11 + i][1];
    }
    for (int i = 0; i < p->cruiser.len; i++)
    {
        p->playerGrid[11 + i][3] = 1;
        p->cruiser.pos[i] = &p->playerGrid[11 + i][3];
    }
    for (int i = 0; i < p->destroyer.len; i++)
    {
        p->playerGrid[11 + i][5] = 1;
        p->destroyer.pos[i] = &p->playerGrid[11 + i][5];
    }
    for (int i = 0; i < p->submarine.len; i++)
    {
        p->playerGrid[11 + i][7] = 1;
        p->submarine.pos[i] = &p->playerGrid[11 + i][7];
    }
    for (int i = 0; i < p->patrolBoat.len; i++)
    {
        p->playerGrid[11 + i][9] = 1;
        p->patrolBoat.pos[i] = &p->playerGrid[11 + i][9];
    }
}

void drawGrid(SDL_Renderer *renderer, int r, int g, int b, int a)
{
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    for (int i = 0; i <= CELL_SIZE * (BOARD_SIZE - 7) + 1; i += CELL_SIZE)
    {
        SDL_RenderDrawLine(renderer, 0, i, CELL_SIZE * (BOARD_SIZE - 7) + 1, i);
        SDL_RenderDrawLine(renderer, i, 0, i, CELL_SIZE * (BOARD_SIZE - 7) + 1);
    }
}

void drawPlayerShips(SDL_Renderer *renderer, player *p)
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (p->playerGrid[i][j] == 1)
            {
                SDL_Rect rect = {i * CELL_SIZE + 1, j * CELL_SIZE + 1, CELL_SIZE - 1, CELL_SIZE - 1};
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
}

void rotateShip(ship *s)
{
    SDL_Event event = getKeypress(FPS);
    if (event.key.keysym.scancode == SDL_SCANCODE_LEFT)
        s->rot = (s->rot + 3) % 4;
    if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT)
        s->rot = (s->rot + 1) % 4;
}

int placeShip(ship *s)
{
    int rot;
    int *startingPos = s->pos[s->len / 2];
    switch (s->rot)
    {
    case 0:
        rot = 17;
        break;
    case 2:
        rot = -17;
        break;
    case 3:
        rot = -1;
        break;
    default:
        rot = s->rot;
        break;
    }
    // check if cells are occupied
    for (int i = 0; i < s->len; i++)
    {
        if (s->pos[i] != startingPos && *(startingPos - (((s->len / 2) - i) * rot)) == 1)
        {
            printf("Can't place ship!\n");
            return 1;
        }
    }
    // place ship w/ rotation
    for (int i = 0; i < s->len; i++)
    {
        if (s->pos[i] != startingPos)
        {
            *s->pos[i] = 0;
            s->pos[i] = startingPos - (((s->len / 2) - i) * rot);
            *s->pos[i] = 1;
        }
    }
    return 0;
}

void takeShot()
{
    // type in coords
    // space to fire, c to clear
}

void render(SDL_Renderer *renderer, player *p)
{
    SDL_Color gridBackground = {22, 22, 22, 255}; // Barely Black
    SDL_Color gridLineColor = {44, 44, 44, 255};  // Dark grey

    drawBackground(renderer, gridBackground.r, gridBackground.g, gridBackground.b, gridBackground.a);
    drawGrid(renderer, gridLineColor.r, gridLineColor.g, gridLineColor.b, gridLineColor.a);
    drawPlayerShips(renderer, p);
    SDL_RenderPresent(renderer);
    SDL_RenderClear(renderer);
}

int main()
{
    SDL_Window *window;
    player p1, p2;

    initializeBoards(&p1);
    initializeBoards(&p2);

    initializeShips(&p1);
    initializeShips(&p2);

    SDL_Renderer *renderer = initializeSDL(window, "Battleship", CELL_SIZE * BOARD_SIZE + 1, CELL_SIZE * BOARD_SIZE + 1);

    render(renderer, &p1);

    while (1)
    {
        rotateShip(&p1.patrolBoat);
        placeShip(&p1.patrolBoat);
        render(renderer, &p1);
    }

    SDL_Delay(10000);

    // players place ships
    // coord == 1
    // store position pointers in ship pointer arrays
    // keep track of player's hits and misses in opponent grid
    // keep track of enemy hits in player grid
    // 1 = ship present
    // 2 = enemy hit
    // check each position pointer array after each hit to see if ship sank

    teardown(renderer, window);

    return 0;
}