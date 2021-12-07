#include <stdio.h>
#include "engine.h"

// initialize SDL events, inputs, window, and renderer
SDL_Renderer *initializeSDL(SDL_Window *window, const char *title, int width, int height)
{
    // Initializes the timer, audio, video, joystick,
    // haptic, gamecontroller and events subsystems
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        printf("Error initializing SDL: %s\n", SDL_GetError());
        return 0;
    }

    // Create a window
    window = SDL_CreateWindow(title,
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              width, height, 0);
    if (!window)
    {
        printf("Error creating window: %s\n", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    // Create a renderer
    Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, render_flags);
    if (!renderer)
    {
        printf("Error creating renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    return renderer;
}

// Release SDL resources
void teardown(SDL_Renderer *renderer, SDL_Window *window)
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

// get input from user
// e.g. keyboard and mouse
SDL_Event getInput()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
        return event;
}

/* Game Mechanics
**********************************************************************
*********************************************************************/

/* Ship Placement
*********************************************************************/
//return offset corresponding to rotation
int getRotation(int rotation)
{
    switch (rotation)
    {
    case 0:
        return BOARD_SIZE_Y;
    case 2:
        return -BOARD_SIZE_Y;
    case 3:
        return -1;
    default:
        return rotation;
    }
}

// check if cells are occupied
bool checkCells(player *p, int *x, int *y, int *rotation, int placing)
{
    ship *s = &p->ships[p->selectedShip];
    int rot, *newPos, newXPos, newYPos, *newCenter = s->pos[s->center];

    // check if center point can be moved
    if (x != NULL && y != NULL)
    {
        if (p->grid[*x][*y] == 1 && s->pos[s->center] != &p->grid[*x][*y])
        {
            printf("Can't place center!\n");
            return false;
        }
        newCenter = &p->grid[*x][*y]; // if cell is clear, set new center equal to pointer to new cell
    }

    // apply rotation if necessary
    if (rotation != NULL)
        rot = getRotation(*rotation);
    else
        rot = getRotation(s->rot);

    // check if rest of ship can be moved
    for (int i = 0; i < s->len; i++)
    {
        newPos = newCenter - ((s->center - i) * rot);
        if (placing)
        {
            newXPos = (newPos - &p->grid[0][0]) / BOARD_SIZE_Y;
            newYPos = (newPos - &p->grid[0][0]) % BOARD_SIZE_Y;
            if (newXPos > 10 || newXPos < 1 || newYPos > 10 || newYPos < 1)
            {
                printf("Can't place ship!\n");
                return false;
            }
        }
        if (s->pos[i] != newCenter && *newPos == 1)
        {
            printf("Can't place ship!\n");
            return false;
        }
    }

    return true;
}

void rotateShip(player *p, int rotation)
{
    // rotation == 3 - CCW
    // rotation == 1 - CW
    int rot = (p->ships[p->selectedShip].rot + rotation) % 4;
    int placing = 0;

    if (p->selectedShip != -1)
    {
        // check cells before applying rotation
        if (p->ships[p->selectedShip].isPlaced)
            placing = 1; // if ship is placed, set placing to 1
        if (checkCells(p, NULL, NULL, &rot, placing))
            p->ships[p->selectedShip].rot = rot;
    }
}

// update position pointer arrays and grids to place ships
// check cells before call placeShips()
void placeShips(player *p)
{
    ship *s;
    int rot;

    for (int i = 0; i < NSHIPS; i++)
    {
        s = &p->ships[i];

        // apply rotation
        rot = getRotation(s->rot);

        // place ship w/ rotation
        for (int j = 0; j < s->len; j++)
        {
            if (s->pos[j] == s->pos[s->center])
            {
                *s->pos[s->center] = 1;
                continue;
            }
            *s->pos[j] = 0;
            s->pos[j] = s->pos[s->center] - ((s->center - j) * rot); // calculate offset from center for each block
            *s->pos[j] = 1;
        }
    }
}

// move selected ship to initial position, right of the board
void clearSelectedShip(player *p)
{
    ship *s = &p->ships[p->selectedShip];
    for (int i = 0; i < s->len; i++)
    {
        *s->pos[i] = 0;
    }
    s->rot = 0;
    s->pos[s->center] = s->initCenterPos;
}

int placeSelectedShip(player *p, int x, int y)
{
    ship *s;
    if (p->selectedShip != -1)
    {
        s = &p->ships[p->selectedShip];
        if (x < 11 && x > 0 && y < 11 && y > 0)
            if (checkCells(p, &x, &y, NULL, 1))
            {
                *s->pos[s->center] = 0;
                s->pos[s->center] = &p->grid[x][y];
                s->isPlaced = true;
                return 0;
            }
        printf("Can't place selected ship!\n");
        return 1;
    }
    return 0;
}

// check that ship can be selected
int setSelectedShip(bool started, player *p, int selectedShip)
{
    ship *s;
    int x, y = 13;

    // check that game has not started and another ship is not selected
    if (!started)
    {
        // preserve placement if previously selected ship was placed
        if (p->selectedShip != -1)
            (p->ships[p->selectedShip].isPlaced) ? p->selectedShip = -1 : clearSelectedShip(p);
        // verify that cells where selected ship is placed are not occupied
        x = 2 + (p->ships[selectedShip].center);
        if (checkCells(p, &x, &y, &p->ships[selectedShip].rot, 0))
        {
            p->selectedShip = selectedShip;
            s = &p->ships[p->selectedShip];
            s->isPlaced = false;
            *s->pos[s->center] = 0;
            s->pos[s->center] = &p->grid[13 + (s->center)][y];
            return 0;
        }
    }
    printf("Can't select ship!\n");
    return 1;
}
/********************************************************************/

/* Shooting
*********************************************************************/
void checkSunk()
{
}

int checkGameOver(player *p)
{
    ship *s;
    for (int i = 0; i < NSHIPS; i++)
    {
        s = &p->ships[i];
        for (int j = 0; j < s->len; j++)
            if (*s->pos[j] == 1)
                return 0;
    }
    return 1;
}

// p1 shoots at p2
int takeShot(player *p1, player *p2, bool *running, int x, int y)
{
    if (x < 11 && x > 0 && y < 22 && y > 11)
    {
        if (p1->grid[x][y] == 2)
        {
            printf("Shot taken already!\n");
            return 0;
        }
        if (p2->grid[x][y - 11] == 0)
        {
            p1->grid[x][y] = 2;
            printf("Miss!\n");
            return 1;
        }
        if (p2->grid[x][y - 11] == 1)
        {
            p1->grid[x][y] = 3;
            p2->grid[x][y - 11] = 3;
            printf("Hit!\n");
            if (checkGameOver(p2))
                *running = false;
            return 1;
        }
    }
    return 0;
}
/********************************************************************/

/*********************************************************************
*********************************************************************/