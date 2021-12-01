
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
#define NSHIPS 5

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
    ship ships[NSHIPS];
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
    // player's ships
    ship s;
    for (int i = 0; i < NSHIPS; i++)
        p->ships[i] = s;

    // set ship lengths
    p->ships[0].len = 5; // carrier
    p->ships[1].len = 4; // cruiser
    p->ships[2].len = 3; // destroyer
    p->ships[3].len = 3; // submarine
    p->ships[4].len = 2; // patrol boat

    for (int i = 0; i < NSHIPS; i++)
    {
        p->ships[i].rot = 0;          // initial orientation up
        p->ships[i].isPlaced = false; // ships not placed on board yet

        // place ships to right of player board
        for (int j = 0; j < p->ships[i].len; j++)
        {
            p->playerGrid[11 + j][i * 2 + 1] = 1;
            p->ships[i].pos[j] = &p->playerGrid[11 + j][i * 2 + 1];
        }
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

int selectShip()
{
    SDL_Event event = getKeypress(FPS);
    switch (event.key.keysym.scancode)
    {
    case SDL_SCANCODE_1:
        return 0;
    case SDL_SCANCODE_2:
        return 1;
    case SDL_SCANCODE_3:
        return 2;
    case SDL_SCANCODE_4:
        return 3;
    case SDL_SCANCODE_5:
        return 4;
    default:
        return -1;
    }
}

int placeShip(int playerGrid[BOARD_SIZE][BOARD_SIZE], ship *s, int x, int y)
{
    int rot;
    int shipCenter = s->len / 2;

    // check if center point can be moved
    if (playerGrid[x][y] == 1 && s->pos[shipCenter] != &playerGrid[x][y])
    {
        printf("Can't place ship!\n");
        return 1;
    }
    // move center point
    *s->pos[shipCenter] = 0;
    s->pos[shipCenter] = &playerGrid[x][y];
    *s->pos[shipCenter] = 1;

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
        if (s->pos[i] != s->pos[shipCenter] && *(s->pos[shipCenter] - (((s->len / 2) - i) * rot)) == 1)
        {
            printf("Can't place ship!\n");
            return 1;
        }
    }
    // place ship w/ rotation
    for (int i = 0; i < s->len; i++)
    {
        if (s->pos[i] != s->pos[shipCenter])
        {
            *s->pos[i] = 0;
            s->pos[i] = s->pos[shipCenter] - (((s->len / 2) - i) * rot);
            *s->pos[i] = 1;
        }
    }
    return 0;
}

// draw objects and display in window
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

// allow player to rotate ships and place on the board
void placePlayerShips(player *p, SDL_Renderer *renderer)
{
    int shipNum = -1;
    while (shipNum == -1)
        shipNum = selectShip();
    placeShip(p->playerGrid, &p->ships[shipNum], 2 + (p->ships[shipNum].len / 2), 13);
    render(renderer, p);
    while (1)
    {
        rotateShip(&p->ships[shipNum]);
        placeShip(p->playerGrid, &p->ships[shipNum], 2 + (p->ships[shipNum].len / 2), 13);
        render(renderer, p);
    }
}

void takeShot()
{
    // type in coords
    // space to fire, c to clear
}

int main()
{
    SDL_Window *window = NULL;
    player p1, p2;

    initializeBoards(&p1);
    initializeBoards(&p2);

    initializeShips(&p1);
    initializeShips(&p2);

    SDL_Renderer *renderer = initializeSDL(window, "Battleship", CELL_SIZE * BOARD_SIZE + 1, CELL_SIZE * BOARD_SIZE + 1);

    render(renderer, &p1);

    placePlayerShips(&p1, renderer);

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

/*  New Game Loop

FPS dependent on Constant Game Speed



use constant fps

initialize 

while(running)
{
    update_game();
    display_game();

    sleep_time = calculate_sleep();
    if (sleep_time > 0)
    {
        sleep(sleep_time);
    }
}

update_game()
    poll input
    update state of objects/grids

display_game()
    draw objects to display
        position, orientation calculations and such done here

important to properly group functions into update_game() and display_game()

*/