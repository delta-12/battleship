#include <time.h>
#include "engine.h"
#include "rendering.h"

/* Initialization
**********************************************************************
*********************************************************************/

// zero out grids
void initializeBoards(player *p)
{
    for (int i = 0; i < BOARD_SIZE_X; i++)
    {
        for (int j = 0; j < BOARD_SIZE_Y; j++)
        {
            p->grid[i][j] = 0;
        }
    }
}

void initializeShips(player *p)
{
    int x, y;

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
        p->ships[i].rot = 0;                      // initial orientation left
        p->ships[i].isPlaced = false;             // ships not placed on board yet
        p->ships[i].center = p->ships[i].len / 2; // calculate ship's center point

        // place ships to right of player board
        for (int j = 0; j < p->ships[i].len; j++)
        {
            x = 13 + j;
            y = i * 2 + 7;
            p->grid[x][y] = 1;
            p->ships[i].pos[j] = &p->grid[x][y];
        }

        p->ships[i].initCenterPos = p->ships[i].pos[p->ships[i].center]; // initial position of ship's center
    }

    p->selectedShip = -1; // no ship selected yet
}

// place opponent's ships at random locations w/ random rotations
void initializeOpponent(player *p)
{
    ship *s;
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

/*********************************************************************
*********************************************************************/

/* Game Logic
**********************************************************************
*********************************************************************/

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

// opponent takes random shot
void opponentShot(player *p1, player *p2, bool *running)
{
    int x, y;
    do
    {
        x = (rand() % 10) + 1;
        y = (rand() % 10) + 12;
    } while (!takeShot(p2, p1, running, x, y));
}

// perform actions corresponding to user input
void handleInput(bool *running, bool *started, int *turn, player *p1, player *p2)
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
            setSelectedShip(*started, p1, 0);
            break;
        case SDL_SCANCODE_2:
            setSelectedShip(*started, p1, 1);
            break;
        case SDL_SCANCODE_3:
            setSelectedShip(*started, p1, 2);
            break;
        case SDL_SCANCODE_4:
            setSelectedShip(*started, p1, 3);
            break;
        case SDL_SCANCODE_5:
            setSelectedShip(*started, p1, 4);
            break;
        case SDL_SCANCODE_LEFT:
            rotateShip(p1, 3);
            break;
        case SDL_SCANCODE_RIGHT:
            rotateShip(p1, 1);
            break;
        case SDL_SCANCODE_S:
            startGame(p1, started);
            break;
        default:
            break;
        }
        break;
    case SDL_MOUSEBUTTONDOWN:
        if (*started)
        {
            if (takeShot(p1, p2, running, event.motion.x / CELL_SIZE, event.motion.y / CELL_SIZE))
                *turn += 1;
            break;
        }
        placeSelectedShip(p1, event.motion.x / CELL_SIZE, event.motion.y / CELL_SIZE);
        break;
    default:
        break;
    }
}

// perform game logic
int updateGame(bool *running, bool *started, int *turn, player *p1, player *p2)
{
    if (*turn % 2 == 1)
    {
        opponentShot(p1, p2, running);
        *turn += 1;
        return 0;
    }
    handleInput(running, started, turn, p1, p2);
    if (!*started)
    {
        placeShips(p1);
    }
    return 0;
}

/*********************************************************************
*********************************************************************/

int main()
{
    SDL_Window *window = NULL;
    player p1, p2; // player is p1 and computer/opponent is p2
    clock_t start, end;
    double sleepTime;
    int turn = 0;
    bool running = true, started = false; // testing

    srand(time(NULL)); // initialize random psuedo-random seed

    initializeBoards(&p1);
    initializeBoards(&p2);

    initializeShips(&p1);
    initializeShips(&p2);

    initializeOpponent(&p2);

    SDL_Renderer *renderer = initializeSDL(window, "Battleship", CELL_SIZE * BOARD_SIZE_X + 1, CELL_SIZE * BOARD_SIZE_Y + 1);

    // testing
    SDL_Window *window2 = NULL;
    SDL_Renderer *renderer2 = initializeSDL(window, "Battleship Opp", CELL_SIZE * BOARD_SIZE_X + 1, CELL_SIZE * BOARD_SIZE_Y + 1);
    render(renderer2, &p2);

    render(renderer, &p1);

    while (running)
    {
        start = clock();
        updateGame(&running, &started, &turn, &p1, &p2);
        render(renderer, &p1);
        render(renderer2, &p2); // testing
        end = clock();

        sleepTime = SKIP_TICKS - ((double)(end - start) / CLOCKS_PER_SEC);
        (sleepTime >= 0) ? SDL_Delay(sleepTime) : printf("Running %lfs behind!\n", -1 * sleepTime); // need to calculate delay instead of fixed delay, adjust FPS
    }

    printf("Game Over!\n");
    SDL_Delay(3000);

    // box selection area
    // keep track of player's hits and misses in opponent grid
    // keep track of enemy hits in player grid
    // check each position pointer array after each hit to see if ship sank
    // mouse motion placement prediction
    // remove printf statements

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

    //testing
    teardown(renderer2, window2);

    teardown(renderer, window);

    return 0;
}
