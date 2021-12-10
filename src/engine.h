#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <SDL2/SDL.h>
#include <stdbool.h>

#define BOARD_SIZE_X 19
#define BOARD_SIZE_Y 23
#define CELL_SIZE 36
#define NSHIPS 5

#define FPS 100
#define SKIP_TICKS 1000 / FPS

// general ship struct
typedef struct
{
    int *pos[5];
    int len, rot; // rotation- 0 left, 1 up, 2 right, 3 down
    int *initCenterPos;
    int center;
    bool isPlaced;
    bool sunk;
} ship;

// general player struct
typedef struct
{
    // player's boards
    int grid[BOARD_SIZE_X][BOARD_SIZE_Y];

    // player's ships
    ship ships[NSHIPS];
    int selectedShip;
} player;

SDL_Renderer *initializeSDL(SDL_Window *window, const char *title, int width, int height);
void teardown(SDL_Renderer *renderer, SDL_Window *window);
SDL_Event getInput();

/* Game Mechanics
**********************************************************************
*********************************************************************/

/* Ship Placement
*********************************************************************/
int getRotation(int rotation);
bool checkCells(player *p, int *x, int *y, int *rotation, int placing);
void rotateShip(player *p, int rotation);
void placeShips(player *p);
void clearSelectedShip(player *p);
int placeSelectedShip(player *p, int x, int y);
int setSelectedShip(bool started, player *p, int selectedShip);
/********************************************************************/

/* Shooting
*********************************************************************/
int checkSunk(player *p, int x, int y);
int checkGameOver(player *p);
int takeShot(player *p1, player *p2, bool *running, int x, int y);
/********************************************************************/

/*********************************************************************
*********************************************************************/

#endif