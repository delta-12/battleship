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

    //Initialize SDL_ttf
    if (TTF_Init() != 0)
    {
        printf("Error initializing SDL_ttf: %s\n", TTF_GetError());
        return 0;
    }

    // Create a window
    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0);
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
    TTF_Quit();
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

// return selected message
const char *message(int msg)
{
    switch (msg)
    {
    case 0:
        return "Game started! Your turn...";
    case 1:
        return "Place all ships to start game!";
    case 2:
        return "Can't select ship!";
    case 3:
        return "Can't place ship!";
    case 4:
        return "Shot already taken!";
    case 5:
        return "Miss!";
    case 6:
        return "Hit!";
    case 7:
        return "Opponent's Ship sunk!";
    case 8:
        return "Game Over!";
    case 9:
        return "Opponent Sank Your Ship!";
    case 10:
        return "Your turn...";
    case 11:
        return "Opponent's turn...";
    case 12:
        return "Place your ships...";
    case 13:
        return "Opponent's Board";
    case 14:
        return "Opponent Hit Your Ship!";
    case 15:
        return "Opponent Missed!";
    case 16:
        return "You won!";
    case 17:
        return "You lost!";
    default:
        return 0;
    }
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

// calculate the x and y position on grid from given position on grid
void calculateOffset(player *p, int *pos, int *x, int *y)
{
    int offset;

    offset = pos - &p->grid[0][0];
    *x = offset / BOARD_SIZE_Y;
    *y = offset % BOARD_SIZE_Y;
}

// check if cells are occupied by another ship
bool checkCells(player *p, int *x, int *y, int *rotation, int placing)
{
    ship *s = &p->ships[p->selectedShip];
    int rot, *newPos, newXPos, newYPos, *newCenter = s->pos[s->center];

    // check if center point can be moved
    if (x != NULL && y != NULL)
    {
        if (p->grid[*x][*y] == 1 && s->pos[s->center] != &p->grid[*x][*y])
        {
            p->msg = 3; // can't place ship msg
            p->pause = true;
            return false;
        }
        newCenter = &p->grid[*x][*y]; // if cell is clear, set new center equal to pointer to new cell
    }

    // apply rotation if necessary
    rot = (rotation != NULL) ? getRotation(*rotation) : getRotation(s->rot);

    // check if rest of ship can be moved
    for (int i = 0; i < s->len; i++)
    {
        newPos = newCenter - ((s->center - i) * rot);
        if (placing)
        {
            // calculate new position
            calculateOffset(p, newPos, &newXPos, &newYPos);

            // ship must be placed within 10 x 10 board
            if (newXPos > 10 || newXPos < 1 || newYPos > 10 || newYPos < 1)
            {
                p->msg = 3; // can't place ship msg
                p->pause = true;
                return false;
            }
        }

        // center if cells are clear
        if (s->pos[i] != newCenter && *newPos == 1)
        {
            p->msg = 3; // can't place ship msg
            p->pause = true;
            return false;
        }
    }

    return true;
}

// increment ship's rotation left/CCW or right/CW
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
    int rot, x, y;

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
                *s->pos[s->center] = 1; // place center of ship
                continue;
            }
            *s->pos[j] = 0;
            s->pos[j] = s->pos[s->center] - ((s->center - j) * rot); // calculate offset from center for each block based on rotation
            *s->pos[j] = 1;
        }

        // update position of texture rectangle
        calculateOffset(p, s->pos[0], &x, &y);
        s->numberRect.x = x * CELL_SIZE + 2;
        s->numberRect.y = y * CELL_SIZE + 2;
    }
}

// move selected ship to initial position, right of the board
void clearSelectedShip(player *p)
{
    ship *s = &p->ships[p->selectedShip];

    // clear ship's current position
    for (int i = 0; i < s->len; i++)
    {
        *s->pos[i] = 0;
    }

    // reset rotation and position of center
    s->rot = 0;
    s->pos[s->center] = s->initCenterPos;
}

// move selected ship to a position on the board
int placeSelectedShip(player *p, int x, int y)
{
    ship *s;

    // check that a ship is selected
    if (p->selectedShip != -1)
    {
        s = &p->ships[p->selectedShip];

        // ship must be placed on 10 x 10 board
        if (x < 11 && x > 0 && y < 11 && y > 0)
            // check that new position on board is clear
            if (checkCells(p, &x, &y, NULL, 1))
            {
                *s->pos[s->center] = 0;
                s->pos[s->center] = &p->grid[x][y]; // move center to new position
                s->isPlaced = true;                 // ship is now placed on board
                return 0;
            }
        p->msg = 3; // can't place ship msg
        p->pause = true;
        return 1;
    }
    return 0;
}

// check that ship can be selected
int setSelectedShip(bool started, player *p, int selectedShip)
{
    ship *s;
    int x = 15, y = 3;

    // check that game has not started and another ship is not selected
    if (!started)
    {
        // preserve placement if previously selected ship was placed
        if (p->selectedShip != -1)
            (p->ships[p->selectedShip].isPlaced) ? p->selectedShip = -1 : clearSelectedShip(p);

        // verify that cells where selected ship is placed are not occupied
        if (checkCells(p, &x, &y, &p->ships[selectedShip].rot, 0))
        {
            p->selectedShip = selectedShip;
            s = &p->ships[p->selectedShip];
            s->isPlaced = false; // if selected, the ship is not currently placed
            *s->pos[s->center] = 0;
            s->pos[s->center] = &p->grid[x][y]; // move center to selected ship box
            return 0;
        }
        p->msg = 2; // can't select ship msg
        p->pause = true;
    }
    return 1;
}
/********************************************************************/

/* Shooting
*********************************************************************/
// checks opponent's ships to see if sunk
int checkSunk(player *p2, player *p1, int x, int y)
{
    ship *s;
    bool correctShip, sunk;

    // find ship in opponent's grid that matches the position of the hit
    // and is sunk
    for (int i = 0; i < NSHIPS; i++)
    {
        s = &p2->ships[i];
        correctShip = false;
        sunk = true;
        if (!s->sunk)
        {
            for (int j = 0; j < s->len; j++)
            {
                // every cell of ship must be hit
                if (*s->pos[j] != 3)
                    sunk = false;

                // the ship must contain the position of the hit
                if (s->pos[j] == &p2->grid[x][y])
                {
                    correctShip = true;
                }
            }
            if (correctShip && sunk)
            {
                s->sunk = true;

                // ship sunk msg
                p1->msg = 7;
                p2->msg = 9;
                p1->pause = true;
                p2->pause = true;

                return 1;
            }
        }
    }
    return 0;
}

// if all of player's ships are sunk, game is over
int checkGameOver(player *p)
{
    ship *s;

    for (int i = 0; i < NSHIPS; i++)
    {
        s = &p->ships[i];
        if (!s->sunk)
            return 0;
    }
    return 1;
}

// p1 shoots at p2
int takeShot(player *p1, player *p2, bool *running, int x, int y)
{
    // shot must be within bottom guessing grid
    if (x < 11 && x > 0 && y < 22 && y > 11)
    {
        // check is player already took shot with matching position
        if (p1->grid[x][y] == 2 || p1->grid[x][y] == 3)
        {
            p1->msg = 4; // shot taken already msg
            p1->pause = true;
            p2->pause = true;
            return 0;
        }

        // display miss
        if (p2->grid[x][y - 11] == 0)
        {
            p1->grid[x][y] = 2;
            p1->msg = 5; // miss msg
            p2->msg = 15;
            p1->pause = true;
            p2->pause = true;
            return 1;
        }

        // display hit
        if (p2->grid[x][y - 11] == 1)
        {
            p1->grid[x][y] = 3;
            p2->grid[x][y - 11] = 3;
            p1->msg = 6; // hit msg
            p2->msg = 14;
            p1->pause = true;
            p2->pause = true;
            if (checkSunk(p2, p1, x, y - 11)) // check is ship was sunk
            {
                if (checkGameOver(p2)) // check if game over if ship was sunk
                    *running = false;
                return 3;
            }
            return 2;
        }
    }
    return 0;
}
/********************************************************************/

/*********************************************************************
*********************************************************************/