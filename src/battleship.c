#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "engine.h"
#include <time.h>

#define BOARD_SIZE 19
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
} ship;

typedef struct
{
    // player's boards
    int playerGrid[BOARD_SIZE][BOARD_SIZE];
    int opponentGrid[BOARD_SIZE - 9][BOARD_SIZE - 9];

    // player's ships
    ship ships[NSHIPS];
    int selectedShip;
} player;

/* Initialization
*********************************************************************/
void initializeBoards(player *p)
{
    // zero out grids
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            p->playerGrid[i][j] = 0;
            if (i < BOARD_SIZE - 8 && i > 0 && j < BOARD_SIZE - 8 && j > 0)
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
        p->ships[i].rot = 0; // initial orientation left
        p->ships[i].isPlaced = false;
        p->ships[i].center = p->ships[i].len / 2; // calculate ship's center point

        // place ships to right of player board
        for (int j = 0; j < p->ships[i].len; j++)
        {
            p->playerGrid[13 + j][i * 2 + 1] = 1;
            p->ships[i].pos[j] = &p->playerGrid[13 + j][i * 2 + 1];
        }

        p->ships[i].initCenterPos = p->ships[i].pos[p->ships[i].len / 2]; // initial position of ship's center
    }

    p->selectedShip = -1; // no ship selected yet
}
/********************************************************************/

/* Game Logic
*********************************************************************/
//return offset corresponding to rotation
int getRotation(int rotation)
{
    switch (rotation)
    {
    case 0:
        return 19;
    case 2:
        return -19;
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
        if (p->playerGrid[*x][*y] == 1 && s->pos[s->center] != &p->playerGrid[*x][*y])
        {
            printf("Can't place center!\n");
            return false;
        }
        newCenter = &p->playerGrid[*x][*y]; // if cell is clear, set new center equal to pointer to new cell
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
            newXPos = (newPos - &p->playerGrid[0][0]) / 19;
            newYPos = (newPos - &p->playerGrid[0][0]) % 19;
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

void takeShot()
{
    // type in coords
    // space to fire, c to clear
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
                s->pos[s->center] = &p->playerGrid[x][y];
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
            s->pos[s->center] = &p->playerGrid[13 + (s->center)][y];
            return 0;
        }
    }
    printf("Can't select ship!\n");
    return 1;
}

// start game if all ships are placed on board
int startGame(player *p, bool *started)
{
    *started = true;
    for (int i = 0; i < NSHIPS; i++)
    {
        if (!p->ships[i].isPlaced)
        {
            *started = false;
            printf("Place all ships to start game!\n");
            return 1;
        }
    }
    p->selectedShip = -1;
    printf("Game started!\n");
    return 0;
}

// perform actions corresponding to user input
void handleInput(bool *running, bool *started, player *p)
{
    SDL_Event event = getInput();
    switch (event.type)
    {
    case SDL_QUIT:
        *running = false;
        break;
    case SDL_KEYDOWN:
        switch (event.key.keysym.scancode)
        {
        case SDL_SCANCODE_1:
            setSelectedShip(*started, p, 0);
            break;
        case SDL_SCANCODE_2:
            setSelectedShip(*started, p, 1);
            break;
        case SDL_SCANCODE_3:
            setSelectedShip(*started, p, 2);
            break;
        case SDL_SCANCODE_4:
            setSelectedShip(*started, p, 3);
            break;
        case SDL_SCANCODE_5:
            setSelectedShip(*started, p, 4);
            break;
        case SDL_SCANCODE_LEFT:
            rotateShip(p, 3);
            break;
        case SDL_SCANCODE_RIGHT:
            rotateShip(p, 1);
            break;
        case SDL_SCANCODE_S:
            startGame(p, started);
            break;
        default:
            break;
        }
        break;
    case SDL_MOUSEBUTTONDOWN:
        placeSelectedShip(p, event.motion.x / CELL_SIZE, event.motion.y / CELL_SIZE);
        // add mouse events for placing ships and taking shots
        break;
    default:
        break;
    }
}

// place opponent's ships at random locations w/ random rotations
void initializeOpponent(player *p)
{
    ship *s;
    srand(time(NULL));
    int x, y;

    for (int i = 0; i < NSHIPS; i++)
    {
        p->selectedShip = i;
        s = &p->ships[p->selectedShip];
        s->rot = rand() % 4;
        do
        {
            x = (rand() % 10) + 1;
            y = (rand() % 10) + 1;
            if (checkCells(p, &x, &y, NULL, 1))
            {
                placeSelectedShip(p, x, y);
                placeShips(p);
            }
        } while (!s->isPlaced);
    }
    p->selectedShip = -1;
}

// perform game logic
void updateGame(bool *running, bool *started, player *p1, player *p2)
{
    handleInput(running, started, p1);
    placeShips(p1);
}
/********************************************************************/

/* Render Game
*********************************************************************/
void drawGrid(SDL_Renderer *renderer, int r, int g, int b, int a)
{
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    for (int i = CELL_SIZE + 1; i <= CELL_SIZE * (BOARD_SIZE - 8) + 1; i += CELL_SIZE)
    {
        // top grid
        SDL_RenderDrawLine(renderer, CELL_SIZE + 1, i, CELL_SIZE * (BOARD_SIZE - 8) + 1, i); // horizontal lines
        SDL_RenderDrawLine(renderer, i, CELL_SIZE + 1, i, CELL_SIZE * (BOARD_SIZE - 8) + 1); // vertical lines

        // bottom grid
        SDL_RenderDrawLine(renderer, CELL_SIZE + 1, i + 11 * CELL_SIZE, CELL_SIZE * (BOARD_SIZE - 8) + 1, i + 11 * CELL_SIZE);
        SDL_RenderDrawLine(renderer, i, CELL_SIZE * 12 + 1, i, CELL_SIZE * (BOARD_SIZE + 2) + 1);
    }
    SDL_RenderDrawLine(renderer, 12 * CELL_SIZE + 1, 0, 12 * CELL_SIZE + 1, CELL_SIZE * (BOARD_SIZE + 3) + 1); // vertical dividing line
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
/********************************************************************/

int main()
{
    SDL_Window *window = NULL;
    player p1, p2; // player is p1 and computer/opponent is p2
    clock_t start, end;
    double sleepTime;
    bool running = true, started = false;

    initializeBoards(&p1);
    initializeBoards(&p2);

    initializeShips(&p1);
    initializeShips(&p2);

    initializeOpponent(&p2);

    SDL_Renderer *renderer = initializeSDL(window, "Battleship", CELL_SIZE * BOARD_SIZE + 1, CELL_SIZE * (BOARD_SIZE + 3) + 1);

    render(renderer, &p1);

    while (running)
    {
        start = clock();
        updateGame(&running, &started, &p1, &p2);
        render(renderer, &p1);
        end = clock();

        sleepTime = SKIP_TICKS - ((double)(end - start) / CLOCKS_PER_SEC);
        (sleepTime >= 0) ? SDL_Delay(sleepTime) : printf("Running %lfs behind!\n", -1 * sleepTime); // need to calculate delay instead of fixed delay, adjust FPS
    }

    // box selection area
    // keep track of player's hits and misses in opponent grid
    // keep track of enemy hits in player grid
    // 1 = ship present
    // 2 = enemy hit
    // check each position pointer array after each hit to see if ship sank

    /* AI
    select random cell
    hit or miss
    if hit
        random cell in 1 of 4 possible directions
        if hit, cell either direction along line
        if miss, select new direction
    if miss
        new random cell
    */

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