#include <time.h>
#include "engine.h"
#include "rendering.h"

// stores data about computer player's guess
typedef struct
{
    int initX, initY;
    int offset1, offset2;
    int rot;
} opponentGuess;

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

// set default values and positions for player's ships
void initializeShips(player *p)
{
    ship s;
    int x, y;

    // initialize ships array w/ empty ship structs
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
        p->ships[i].sunk = false;                 // no ships sunk yet
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

// set opponentGuess struct to default values
void resetOpGuess(opponentGuess *opGuess)
{
    opGuess->initX = 0;
    opGuess->initY = 0;
    opGuess->offset1 = 0;
    opGuess->offset2 = 0;
    opGuess->rot = -1;
}

// place opponent's ships at random locations w/ random rotations
void initializeOpponent(player *p, opponentGuess *opGuess)
{
    ship *s;
    int x, y;

    for (int i = 0; i < NSHIPS; i++)
    {
        p->selectedShip = i;
        s = &p->ships[p->selectedShip];
        s->rot = rand() % 4; // randomize rotation
        do
        {
            // randomize position
            x = (rand() % 10) + 1;
            y = (rand() % 10) + 1;

            if (checkCells(p, &x, &y, NULL, 1)) // check that position on board is clear
            {
                // place ships and set grid values
                placeSelectedShip(p, x, y);
                placeShips(p);
            }
        } while (!s->isPlaced);
    }
    p->selectedShip = -1;

    resetOpGuess(opGuess);
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
        // check if ship is placed
        if (!p->ships[i].isPlaced)
        {
            *started = false;
            printf("Place all ships to start game!\n");
            return 1;
        }
    }
    p->selectedShip = -1; // ships cannot be selected after game has started
    printf("Game started!\n");
    return 0;
}

// computer player's shooting logic
void opponentShot(player *p1, player *p2, bool *running, opponentGuess *opGuess)
{
    int x, y, rot, offset, action, i = 0;

    // check if computer player previously had a hit
    if (opGuess->initX != 0 && opGuess->initY != 0)
    {
        do
        {
            // get initial hit
            x = opGuess->initX;
            y = opGuess->initY;

            // determine direction (left/right or up/down) of shot
            // offset1- initial direction, offset2- opposite direction of offset1
            offset = opGuess->offset1 + 1;
            if (opGuess->offset2 > 0)
                offset = opGuess->offset2 * -1;

            // determine ship's rotation and add offset to get shot position
            if (opGuess->rot == -1)
                opGuess->rot = rand() % 4; // assign random rotation if unknown
            rot = opGuess->rot;
            switch (opGuess->rot)
            {
            case 0:
                x = opGuess->initX - offset;
                break;
            case 1:
                y = opGuess->initY - offset;
                break;
            case 2:
                x = opGuess->initX + offset;
                break;
            case 3:
                y = opGuess->initY + offset;
                break;
            }

            action = takeShot(p2, p1, running, x, y);

            switch (action)
            {
            case 2:

                // continue guessing in current direction if shot is a hit
                if (opGuess->offset2 > 0)
                    opGuess->offset2 += 1;
                else
                    opGuess->offset1 += 1;

                break;
            case 3:

                // next shot is random if ship is sunk
                resetOpGuess(opGuess);

                break;
            default:

                // incremenet if shot following initial is a miss
                if (opGuess->offset1 == 0)
                {
                    opGuess->rot = (opGuess->rot + 1) % 4;

                    // guess in opposite direction if shots in each direction have already been taken and/or are misses
                    if (opGuess->rot == rot)
                        opGuess->offset2 = 1;
                }

                else
                    opGuess->offset2 = 1; // guess in opposite direction
                break;
            }
            i++;
        } while (action == 0 && i < 4);

        if (i >= 4)
        {
            // take random guess if more than four consecutive computer player's shot have already been taken
            resetOpGuess(opGuess);
            opponentShot(p1, p2, running, opGuess);
        }
    }

    // take random shot if computer player did not have previous hit
    else
    {
        do
        {
            x = (rand() % 10) + 1;
            y = (rand() % 10) + 12;
            action = takeShot(p2, p1, running, x, y);
        } while (action == 0);
        if (action == 2)
        {
            // keep track of position if shot is a hit
            opGuess->initX = x;
            opGuess->initY = y;
        }
    }
}

// perform actions corresponding to user input
void handleInput(bool *running, bool *started, int *turn, player *p1, player *p2)
{
    SDL_Event event = getInput();
    int x, y, action;

    switch (event.type)
    {
    case SDL_QUIT:
        *running = false;
        break;

    // keyboard input
    case SDL_KEYDOWN:
        switch (event.key.keysym.scancode)
        {

        // select ships 1 - 5 w/ num keys
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

        // rotate ships left or right w/ arrow keys
        case SDL_SCANCODE_LEFT:
            rotateShip(p1, 3);
            break;
        case SDL_SCANCODE_RIGHT:
            rotateShip(p1, 1);
            break;

        // start game by pressing s
        case SDL_SCANCODE_S:
            startGame(p1, started);
            break;

        default:
            break;
        }
        break;

    // mouse input
    case SDL_MOUSEBUTTONDOWN:

        // get position of mouse on grid
        x = event.motion.x / CELL_SIZE;
        y = event.motion.y / CELL_SIZE;

        // take shot at position of mouse click if game has started
        if (*started)
        {
            action = takeShot(p1, p2, running, x, y);
            if (action != 0)
                *turn += 1;
            break;
        }

        // place ship at position of mouse click if game has not started
        placeSelectedShip(p1, x, y);
        break;

    default:
        break;
    }
}

// perform game logic
int updateGame(bool *running, bool *started, int *turn, player *p1, player *p2, opponentGuess *opGuess)
{
    // computer plays on odd turn number
    if (*turn % 2 == 1)
    {
        opponentShot(p1, p2, running, opGuess);
        *turn += 1;
        return 0;
    }

    // player plays on even turn number
    handleInput(running, started, turn, p1, p2);
    if (!*started)
    {
        placeShips(p1); // place player's ships on grid if game has not started
    }
    return 0;
}

/*********************************************************************
*********************************************************************/

int main()
{
    SDL_Window *window = NULL;
    player p1, p2; // player is p1 and computer/opponent is p2
    opponentGuess opGuess;
    clock_t start, end;
    double sleepTime;
    int turn = 0;
    bool running = true, started = false;

    srand(time(NULL)); // initialize random psuedo-random seed for random actions

    // initialize game elements
    initializeBoards(&p1);
    initializeBoards(&p2);
    initializeShips(&p1);
    initializeShips(&p2);
    initializeOpponent(&p2, &opGuess);

    // create game windows and renderer
    SDL_Renderer *renderer = initializeSDL(window, "Battleship", CELL_SIZE * BOARD_SIZE_X + 1, CELL_SIZE * BOARD_SIZE_Y + 1);

    // display blank board
    render(renderer, &p1);

    // game loop
    // FPS dependent on constant game speed
    while (running)
    {
        // time exectuion of game logic and rendering
        start = clock();
        updateGame(&running, &started, &turn, &p1, &p2, &opGuess);
        render(renderer, &p1);
        end = clock();

        sleepTime = SKIP_TICKS - ((double)(end - start) / CLOCKS_PER_SEC); // calculate time to sleep between rendering each frame
        (sleepTime >= 0) ? SDL_Delay(sleepTime) : printf("Running %lfs behind!\n", -1 * sleepTime);
    }

    printf("Game Over!\n");
    SDL_Delay(3000);

    // reveal computer player's board at end of game
    render(renderer, &p2);
    SDL_Delay(10000);

    teardown(renderer, window);

    return 0;
}

// TODO
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
    keep track of last hit until ship has sank, then reset
*/