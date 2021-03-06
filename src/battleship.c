/*
Welcome to Battleship!

Objective: Sink all of your opponent's ships.

In this game, you will play against the computer to see who can sink all of each other's ships first.

1. Placing ships
    The first step is to place all of your ships on the top grid.  This can be done by pressing the number key that corresponds to the number on the ship, using the arrow keys to rotate the ship to desired orientation, and then clicking the cell in the top grid that you want to place the center of the ship in.
    Ships must be placed completely within the top grid.  If the cell at which the center of ship is to be placed causes the ship to overhang the edge of the top grid, then a message will be displayed, alerting you that you cannot place the ship there.
    Each ship has a given length corresponding to the number of cells it fills.
        1. Aircraft Carrier - 5 cells
        2. Cruiser - 4 cells
        3. Destroyer - 3 cells
        4. Submarine - 3 cells
        5. Patrol Boat - 2 cells
    The location of a ship on the grid is the cells it fills when placed.  The ships are colored in green.
    All of your ships combined comprise your fleet.
    Your bottom grid corresponds to your opponent's top grid, where its ships are placed, and vice versa.

2. Playing the game
    When all of your ships have been placed, press the 's' key to start the game.
    You and your computer opponent will alternate taking "shots" at each other's ships.  The idea is that you are taking shooting at an enemy battleships, trying to sink them.  Each shot is guess at the location of your opponent's ships.  If the location of a shot matches a cell filled by one of your opponent's ships, then it is said to be a "hit".  If not, is a "miss".
    To take a shot, click on a cell in the bottom grid that you think is filled by one of your opponent's ships.  If it is a hit, it will be marked in red.  If it is a miss, it will be marked in white.  Your opponent's hits on your ships will also be marked in red on your top grid.
    When all the cells filled by a ship have been hit, or in other words, when an entire ship's location has been guessed, the ship is said to be sunk.  A message at the bottom of the screen will indicate if a ship has been sunk after the final shot needed to sink the ship was taken.
    When it is your turn, you will shoot at your opponent's ships.  Then, it is your opponent's turn, and it will do the same.  This process repeats until the game is over.

3. Winning the game
    The game is over after a player's fleet, i.e. all of a player's ships, have been sunk.  The winner is whoever is able to sink the other player's fleet first.
    Your computer opponent's board will also be revealed at the end of the game.
*/

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

        // set rectangle dimensions for copying number texture
        p->ships[i].numberRect.w = 13;
        p->ships[i].numberRect.h = 25;

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
            p->msg = 1; // place all ships to start game msg
            p->pause = true;
            return 1;
        }
    }
    p->selectedShip = -1; // ships cannot be selected after game has started
    p->msg = 0;           // game started msg
    return 0;
}

/* computer player's shooting logic
    select random cell
    hit or miss
    if hit
        random cell in 1 of 4 possible directions
        if hit, gusss in cells either direction along line
        if miss, select new direction
    if miss
        new random cell
    keep track of last hit until ship has sank, then reset
*/
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
            offset = (opGuess->offset2 > 0) ? opGuess->offset2 * -1 : opGuess->offset1 + 1;

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

    if (!p1->pause)
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
        if (!p1->pause)
        {
            opponentShot(p1, p2, running, opGuess);
            *turn += 1;
            p2->pause = false;
            return 0;
        }
    }

    // player plays on even turn number
    handleInput(running, started, turn, p1, p2);
    if (*started)
    {
        if (*turn != 0 && !p1->pause)
            p1->msg = 10; // your turn msg
    }
    else
    {
        p1->msg = (p1->pause) ? p1->msg : 12; // place your ships msg
        placeShips(p1);                       // place player's ships on grid if game has not started
    }
    return 0;
}

/*********************************************************************
*********************************************************************/

/* Instructions msg
**********************************************************************
*********************************************************************/
// display instructions
void displayInstructions()
{

    unsigned char resources_instructions_txt[] = {
        0x57, 0x65, 0x6c, 0x63, 0x6f, 0x6d, 0x65, 0x20, 0x74, 0x6f, 0x20, 0x42,
        0x61, 0x74, 0x74, 0x6c, 0x65, 0x73, 0x68, 0x69, 0x70, 0x21, 0x0a, 0x0a,
        0x4f, 0x62, 0x6a, 0x65, 0x63, 0x74, 0x69, 0x76, 0x65, 0x3a, 0x20, 0x53,
        0x69, 0x6e, 0x6b, 0x20, 0x61, 0x6c, 0x6c, 0x20, 0x6f, 0x66, 0x20, 0x79,
        0x6f, 0x75, 0x72, 0x20, 0x6f, 0x70, 0x70, 0x6f, 0x6e, 0x65, 0x6e, 0x74,
        0x27, 0x73, 0x20, 0x73, 0x68, 0x69, 0x70, 0x73, 0x2e, 0x0a, 0x0a, 0x49,
        0x6e, 0x20, 0x74, 0x68, 0x69, 0x73, 0x20, 0x67, 0x61, 0x6d, 0x65, 0x2c,
        0x20, 0x79, 0x6f, 0x75, 0x20, 0x77, 0x69, 0x6c, 0x6c, 0x20, 0x70, 0x6c,
        0x61, 0x79, 0x20, 0x61, 0x67, 0x61, 0x69, 0x6e, 0x73, 0x74, 0x20, 0x74,
        0x68, 0x65, 0x20, 0x63, 0x6f, 0x6d, 0x70, 0x75, 0x74, 0x65, 0x72, 0x20,
        0x74, 0x6f, 0x20, 0x73, 0x65, 0x65, 0x20, 0x77, 0x68, 0x6f, 0x20, 0x63,
        0x61, 0x6e, 0x20, 0x73, 0x69, 0x6e, 0x6b, 0x20, 0x61, 0x6c, 0x6c, 0x20,
        0x6f, 0x66, 0x20, 0x65, 0x61, 0x63, 0x68, 0x20, 0x6f, 0x74, 0x68, 0x65,
        0x72, 0x27, 0x73, 0x20, 0x73, 0x68, 0x69, 0x70, 0x73, 0x20, 0x66, 0x69,
        0x72, 0x73, 0x74, 0x2e, 0x0a, 0x0a, 0x31, 0x2e, 0x20, 0x50, 0x6c, 0x61,
        0x63, 0x69, 0x6e, 0x67, 0x20, 0x73, 0x68, 0x69, 0x70, 0x73, 0x0a, 0x54,
        0x68, 0x65, 0x20, 0x66, 0x69, 0x72, 0x73, 0x74, 0x20, 0x73, 0x74, 0x65,
        0x70, 0x20, 0x69, 0x73, 0x20, 0x74, 0x6f, 0x20, 0x70, 0x6c, 0x61, 0x63,
        0x65, 0x20, 0x61, 0x6c, 0x6c, 0x20, 0x6f, 0x66, 0x20, 0x79, 0x6f, 0x75,
        0x72, 0x20, 0x73, 0x68, 0x69, 0x70, 0x73, 0x20, 0x6f, 0x6e, 0x20, 0x74,
        0x68, 0x65, 0x20, 0x74, 0x6f, 0x70, 0x20, 0x67, 0x72, 0x69, 0x64, 0x2e,
        0x20, 0x20, 0x54, 0x68, 0x69, 0x73, 0x20, 0x63, 0x61, 0x6e, 0x20, 0x62,
        0x65, 0x20, 0x64, 0x6f, 0x6e, 0x65, 0x20, 0x62, 0x79, 0x20, 0x70, 0x72,
        0x65, 0x73, 0x73, 0x69, 0x6e, 0x67, 0x20, 0x74, 0x68, 0x65, 0x20, 0x6e,
        0x75, 0x6d, 0x62, 0x65, 0x72, 0x20, 0x6b, 0x65, 0x79, 0x20, 0x74, 0x68,
        0x61, 0x74, 0x20, 0x63, 0x6f, 0x72, 0x72, 0x65, 0x73, 0x70, 0x6f, 0x6e,
        0x64, 0x73, 0x20, 0x74, 0x6f, 0x20, 0x74, 0x68, 0x65, 0x20, 0x6e, 0x75,
        0x6d, 0x62, 0x65, 0x72, 0x20, 0x6f, 0x6e, 0x20, 0x74, 0x68, 0x65, 0x20,
        0x73, 0x68, 0x69, 0x70, 0x2c, 0x20, 0x75, 0x73, 0x69, 0x6e, 0x67, 0x20,
        0x74, 0x68, 0x65, 0x20, 0x61, 0x72, 0x72, 0x6f, 0x77, 0x20, 0x6b, 0x65,
        0x79, 0x73, 0x20, 0x74, 0x6f, 0x20, 0x72, 0x6f, 0x74, 0x61, 0x74, 0x65,
        0x20, 0x74, 0x68, 0x65, 0x20, 0x73, 0x68, 0x69, 0x70, 0x20, 0x74, 0x6f,
        0x20, 0x64, 0x65, 0x73, 0x69, 0x72, 0x65, 0x64, 0x20, 0x6f, 0x72, 0x69,
        0x65, 0x6e, 0x74, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x2c, 0x20, 0x61, 0x6e,
        0x64, 0x20, 0x74, 0x68, 0x65, 0x6e, 0x20, 0x63, 0x6c, 0x69, 0x63, 0x6b,
        0x69, 0x6e, 0x67, 0x20, 0x74, 0x68, 0x65, 0x20, 0x63, 0x65, 0x6c, 0x6c,
        0x20, 0x69, 0x6e, 0x20, 0x74, 0x68, 0x65, 0x20, 0x74, 0x6f, 0x70, 0x20,
        0x67, 0x72, 0x69, 0x64, 0x20, 0x74, 0x68, 0x61, 0x74, 0x20, 0x79, 0x6f,
        0x75, 0x20, 0x77, 0x61, 0x6e, 0x74, 0x20, 0x74, 0x6f, 0x20, 0x70, 0x6c,
        0x61, 0x63, 0x65, 0x20, 0x74, 0x68, 0x65, 0x20, 0x63, 0x65, 0x6e, 0x74,
        0x65, 0x72, 0x20, 0x6f, 0x66, 0x20, 0x74, 0x68, 0x65, 0x20, 0x73, 0x68,
        0x69, 0x70, 0x20, 0x69, 0x6e, 0x2e, 0x0a, 0x53, 0x68, 0x69, 0x70, 0x73,
        0x20, 0x6d, 0x75, 0x73, 0x74, 0x20, 0x62, 0x65, 0x20, 0x70, 0x6c, 0x61,
        0x63, 0x65, 0x64, 0x20, 0x63, 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65,
        0x6c, 0x79, 0x20, 0x77, 0x69, 0x74, 0x68, 0x69, 0x6e, 0x20, 0x74, 0x68,
        0x65, 0x20, 0x74, 0x6f, 0x70, 0x20, 0x67, 0x72, 0x69, 0x64, 0x2e, 0x20,
        0x20, 0x49, 0x66, 0x20, 0x74, 0x68, 0x65, 0x20, 0x63, 0x65, 0x6c, 0x6c,
        0x20, 0x61, 0x74, 0x20, 0x77, 0x68, 0x69, 0x63, 0x68, 0x20, 0x74, 0x68,
        0x65, 0x20, 0x63, 0x65, 0x6e, 0x74, 0x65, 0x72, 0x20, 0x6f, 0x66, 0x20,
        0x73, 0x68, 0x69, 0x70, 0x20, 0x69, 0x73, 0x20, 0x74, 0x6f, 0x20, 0x62,
        0x65, 0x20, 0x70, 0x6c, 0x61, 0x63, 0x65, 0x64, 0x20, 0x63, 0x61, 0x75,
        0x73, 0x65, 0x73, 0x20, 0x74, 0x68, 0x65, 0x20, 0x73, 0x68, 0x69, 0x70,
        0x20, 0x74, 0x6f, 0x20, 0x6f, 0x76, 0x65, 0x72, 0x68, 0x61, 0x6e, 0x67,
        0x20, 0x74, 0x68, 0x65, 0x20, 0x65, 0x64, 0x67, 0x65, 0x20, 0x6f, 0x66,
        0x20, 0x74, 0x68, 0x65, 0x20, 0x74, 0x6f, 0x70, 0x20, 0x67, 0x72, 0x69,
        0x64, 0x2c, 0x20, 0x74, 0x68, 0x65, 0x6e, 0x20, 0x61, 0x20, 0x6d, 0x65,
        0x73, 0x73, 0x61, 0x67, 0x65, 0x20, 0x77, 0x69, 0x6c, 0x6c, 0x20, 0x62,
        0x65, 0x20, 0x64, 0x69, 0x73, 0x70, 0x6c, 0x61, 0x79, 0x65, 0x64, 0x2c,
        0x20, 0x61, 0x6c, 0x65, 0x72, 0x74, 0x69, 0x6e, 0x67, 0x20, 0x79, 0x6f,
        0x75, 0x20, 0x74, 0x68, 0x61, 0x74, 0x20, 0x79, 0x6f, 0x75, 0x20, 0x63,
        0x61, 0x6e, 0x6e, 0x6f, 0x74, 0x20, 0x70, 0x6c, 0x61, 0x63, 0x65, 0x20,
        0x74, 0x68, 0x65, 0x20, 0x73, 0x68, 0x69, 0x70, 0x20, 0x74, 0x68, 0x65,
        0x72, 0x65, 0x2e, 0x0a, 0x45, 0x61, 0x63, 0x68, 0x20, 0x73, 0x68, 0x69,
        0x70, 0x20, 0x68, 0x61, 0x73, 0x20, 0x61, 0x20, 0x67, 0x69, 0x76, 0x65,
        0x6e, 0x20, 0x6c, 0x65, 0x6e, 0x67, 0x74, 0x68, 0x20, 0x63, 0x6f, 0x72,
        0x72, 0x65, 0x73, 0x70, 0x6f, 0x6e, 0x64, 0x69, 0x6e, 0x67, 0x20, 0x74,
        0x6f, 0x20, 0x74, 0x68, 0x65, 0x20, 0x6e, 0x75, 0x6d, 0x62, 0x65, 0x72,
        0x20, 0x6f, 0x66, 0x20, 0x63, 0x65, 0x6c, 0x6c, 0x73, 0x20, 0x69, 0x74,
        0x20, 0x66, 0x69, 0x6c, 0x6c, 0x73, 0x2e, 0x0a, 0x20, 0x20, 0x20, 0x20,
        0x31, 0x2e, 0x20, 0x41, 0x69, 0x72, 0x63, 0x72, 0x61, 0x66, 0x74, 0x20,
        0x43, 0x61, 0x72, 0x72, 0x69, 0x65, 0x72, 0x20, 0x2d, 0x20, 0x35, 0x20,
        0x63, 0x65, 0x6c, 0x6c, 0x73, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x32, 0x2e,
        0x20, 0x43, 0x72, 0x75, 0x69, 0x73, 0x65, 0x72, 0x20, 0x2d, 0x20, 0x34,
        0x20, 0x63, 0x65, 0x6c, 0x6c, 0x73, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x33,
        0x2e, 0x20, 0x44, 0x65, 0x73, 0x74, 0x72, 0x6f, 0x79, 0x65, 0x72, 0x20,
        0x2d, 0x20, 0x33, 0x20, 0x63, 0x65, 0x6c, 0x6c, 0x73, 0x0a, 0x20, 0x20,
        0x20, 0x20, 0x34, 0x2e, 0x20, 0x53, 0x75, 0x62, 0x6d, 0x61, 0x72, 0x69,
        0x6e, 0x65, 0x20, 0x2d, 0x20, 0x33, 0x20, 0x63, 0x65, 0x6c, 0x6c, 0x73,
        0x0a, 0x20, 0x20, 0x20, 0x20, 0x35, 0x2e, 0x20, 0x50, 0x61, 0x74, 0x72,
        0x6f, 0x6c, 0x20, 0x42, 0x6f, 0x61, 0x74, 0x20, 0x2d, 0x20, 0x32, 0x20,
        0x63, 0x65, 0x6c, 0x6c, 0x73, 0x0a, 0x54, 0x68, 0x65, 0x20, 0x6c, 0x6f,
        0x63, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x6f, 0x66, 0x20, 0x61, 0x20,
        0x73, 0x68, 0x69, 0x70, 0x20, 0x6f, 0x6e, 0x20, 0x74, 0x68, 0x65, 0x20,
        0x67, 0x72, 0x69, 0x64, 0x20, 0x69, 0x73, 0x20, 0x74, 0x68, 0x65, 0x20,
        0x63, 0x65, 0x6c, 0x6c, 0x73, 0x20, 0x69, 0x74, 0x20, 0x66, 0x69, 0x6c,
        0x6c, 0x73, 0x20, 0x77, 0x68, 0x65, 0x6e, 0x20, 0x70, 0x6c, 0x61, 0x63,
        0x65, 0x64, 0x2e, 0x20, 0x20, 0x54, 0x68, 0x65, 0x20, 0x73, 0x68, 0x69,
        0x70, 0x73, 0x20, 0x61, 0x72, 0x65, 0x20, 0x63, 0x6f, 0x6c, 0x6f, 0x72,
        0x65, 0x64, 0x20, 0x69, 0x6e, 0x20, 0x67, 0x72, 0x65, 0x65, 0x6e, 0x2e,
        0x0a, 0x41, 0x6c, 0x6c, 0x20, 0x6f, 0x66, 0x20, 0x79, 0x6f, 0x75, 0x72,
        0x20, 0x73, 0x68, 0x69, 0x70, 0x73, 0x20, 0x63, 0x6f, 0x6d, 0x62, 0x69,
        0x6e, 0x65, 0x64, 0x20, 0x63, 0x6f, 0x6d, 0x70, 0x72, 0x69, 0x73, 0x65,
        0x20, 0x79, 0x6f, 0x75, 0x72, 0x20, 0x66, 0x6c, 0x65, 0x65, 0x74, 0x2e,
        0x0a, 0x59, 0x6f, 0x75, 0x72, 0x20, 0x62, 0x6f, 0x74, 0x74, 0x6f, 0x6d,
        0x20, 0x67, 0x72, 0x69, 0x64, 0x20, 0x63, 0x6f, 0x72, 0x72, 0x65, 0x73,
        0x70, 0x6f, 0x6e, 0x64, 0x73, 0x20, 0x74, 0x6f, 0x20, 0x79, 0x6f, 0x75,
        0x72, 0x20, 0x6f, 0x70, 0x70, 0x6f, 0x6e, 0x65, 0x6e, 0x74, 0x27, 0x73,
        0x20, 0x74, 0x6f, 0x70, 0x20, 0x67, 0x72, 0x69, 0x64, 0x2c, 0x20, 0x77,
        0x68, 0x65, 0x72, 0x65, 0x20, 0x69, 0x74, 0x73, 0x20, 0x73, 0x68, 0x69,
        0x70, 0x73, 0x20, 0x61, 0x72, 0x65, 0x20, 0x70, 0x6c, 0x61, 0x63, 0x65,
        0x64, 0x2c, 0x20, 0x61, 0x6e, 0x64, 0x20, 0x76, 0x69, 0x63, 0x65, 0x20,
        0x76, 0x65, 0x72, 0x73, 0x61, 0x2e, 0x0a, 0x0a, 0x32, 0x2e, 0x20, 0x50,
        0x6c, 0x61, 0x79, 0x69, 0x6e, 0x67, 0x20, 0x74, 0x68, 0x65, 0x20, 0x67,
        0x61, 0x6d, 0x65, 0x0a, 0x57, 0x68, 0x65, 0x6e, 0x20, 0x61, 0x6c, 0x6c,
        0x20, 0x6f, 0x66, 0x20, 0x79, 0x6f, 0x75, 0x72, 0x20, 0x73, 0x68, 0x69,
        0x70, 0x73, 0x20, 0x68, 0x61, 0x76, 0x65, 0x20, 0x62, 0x65, 0x65, 0x6e,
        0x20, 0x70, 0x6c, 0x61, 0x63, 0x65, 0x64, 0x2c, 0x20, 0x70, 0x72, 0x65,
        0x73, 0x73, 0x20, 0x74, 0x68, 0x65, 0x20, 0x27, 0x73, 0x27, 0x20, 0x6b,
        0x65, 0x79, 0x20, 0x74, 0x6f, 0x20, 0x73, 0x74, 0x61, 0x72, 0x74, 0x20,
        0x74, 0x68, 0x65, 0x20, 0x67, 0x61, 0x6d, 0x65, 0x2e, 0x0a, 0x59, 0x6f,
        0x75, 0x20, 0x61, 0x6e, 0x64, 0x20, 0x79, 0x6f, 0x75, 0x72, 0x20, 0x63,
        0x6f, 0x6d, 0x70, 0x75, 0x74, 0x65, 0x72, 0x20, 0x6f, 0x70, 0x70, 0x6f,
        0x6e, 0x65, 0x6e, 0x74, 0x20, 0x77, 0x69, 0x6c, 0x6c, 0x20, 0x61, 0x6c,
        0x74, 0x65, 0x72, 0x6e, 0x61, 0x74, 0x65, 0x20, 0x74, 0x61, 0x6b, 0x69,
        0x6e, 0x67, 0x20, 0x22, 0x73, 0x68, 0x6f, 0x74, 0x73, 0x22, 0x20, 0x61,
        0x74, 0x20, 0x65, 0x61, 0x63, 0x68, 0x20, 0x6f, 0x74, 0x68, 0x65, 0x72,
        0x27, 0x73, 0x20, 0x73, 0x68, 0x69, 0x70, 0x73, 0x2e, 0x20, 0x20, 0x54,
        0x68, 0x65, 0x20, 0x69, 0x64, 0x65, 0x61, 0x20, 0x69, 0x73, 0x20, 0x74,
        0x68, 0x61, 0x74, 0x20, 0x79, 0x6f, 0x75, 0x20, 0x61, 0x72, 0x65, 0x20,
        0x74, 0x61, 0x6b, 0x69, 0x6e, 0x67, 0x20, 0x73, 0x68, 0x6f, 0x6f, 0x74,
        0x69, 0x6e, 0x67, 0x20, 0x61, 0x74, 0x20, 0x61, 0x6e, 0x20, 0x65, 0x6e,
        0x65, 0x6d, 0x79, 0x20, 0x62, 0x61, 0x74, 0x74, 0x6c, 0x65, 0x73, 0x68,
        0x69, 0x70, 0x73, 0x2c, 0x20, 0x74, 0x72, 0x79, 0x69, 0x6e, 0x67, 0x20,
        0x74, 0x6f, 0x20, 0x73, 0x69, 0x6e, 0x6b, 0x20, 0x74, 0x68, 0x65, 0x6d,
        0x2e, 0x20, 0x20, 0x45, 0x61, 0x63, 0x68, 0x20, 0x73, 0x68, 0x6f, 0x74,
        0x20, 0x69, 0x73, 0x20, 0x67, 0x75, 0x65, 0x73, 0x73, 0x20, 0x61, 0x74,
        0x20, 0x74, 0x68, 0x65, 0x20, 0x6c, 0x6f, 0x63, 0x61, 0x74, 0x69, 0x6f,
        0x6e, 0x20, 0x6f, 0x66, 0x20, 0x79, 0x6f, 0x75, 0x72, 0x20, 0x6f, 0x70,
        0x70, 0x6f, 0x6e, 0x65, 0x6e, 0x74, 0x27, 0x73, 0x20, 0x73, 0x68, 0x69,
        0x70, 0x73, 0x2e, 0x20, 0x20, 0x49, 0x66, 0x20, 0x74, 0x68, 0x65, 0x20,
        0x6c, 0x6f, 0x63, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x6f, 0x66, 0x20,
        0x61, 0x20, 0x73, 0x68, 0x6f, 0x74, 0x20, 0x6d, 0x61, 0x74, 0x63, 0x68,
        0x65, 0x73, 0x20, 0x61, 0x20, 0x63, 0x65, 0x6c, 0x6c, 0x20, 0x66, 0x69,
        0x6c, 0x6c, 0x65, 0x64, 0x20, 0x62, 0x79, 0x20, 0x6f, 0x6e, 0x65, 0x20,
        0x6f, 0x66, 0x20, 0x79, 0x6f, 0x75, 0x72, 0x20, 0x6f, 0x70, 0x70, 0x6f,
        0x6e, 0x65, 0x6e, 0x74, 0x27, 0x73, 0x20, 0x73, 0x68, 0x69, 0x70, 0x73,
        0x2c, 0x20, 0x74, 0x68, 0x65, 0x6e, 0x20, 0x69, 0x74, 0x20, 0x69, 0x73,
        0x20, 0x73, 0x61, 0x69, 0x64, 0x20, 0x74, 0x6f, 0x20, 0x62, 0x65, 0x20,
        0x61, 0x20, 0x22, 0x68, 0x69, 0x74, 0x22, 0x2e, 0x20, 0x20, 0x49, 0x66,
        0x20, 0x6e, 0x6f, 0x74, 0x2c, 0x20, 0x69, 0x73, 0x20, 0x61, 0x20, 0x22,
        0x6d, 0x69, 0x73, 0x73, 0x22, 0x2e, 0x0a, 0x54, 0x6f, 0x20, 0x74, 0x61,
        0x6b, 0x65, 0x20, 0x61, 0x20, 0x73, 0x68, 0x6f, 0x74, 0x2c, 0x20, 0x63,
        0x6c, 0x69, 0x63, 0x6b, 0x20, 0x6f, 0x6e, 0x20, 0x61, 0x20, 0x63, 0x65,
        0x6c, 0x6c, 0x20, 0x69, 0x6e, 0x20, 0x74, 0x68, 0x65, 0x20, 0x62, 0x6f,
        0x74, 0x74, 0x6f, 0x6d, 0x20, 0x67, 0x72, 0x69, 0x64, 0x20, 0x74, 0x68,
        0x61, 0x74, 0x20, 0x79, 0x6f, 0x75, 0x20, 0x74, 0x68, 0x69, 0x6e, 0x6b,
        0x20, 0x69, 0x73, 0x20, 0x66, 0x69, 0x6c, 0x6c, 0x65, 0x64, 0x20, 0x62,
        0x79, 0x20, 0x6f, 0x6e, 0x65, 0x20, 0x6f, 0x66, 0x20, 0x79, 0x6f, 0x75,
        0x72, 0x20, 0x6f, 0x70, 0x70, 0x6f, 0x6e, 0x65, 0x6e, 0x74, 0x27, 0x73,
        0x20, 0x73, 0x68, 0x69, 0x70, 0x73, 0x2e, 0x20, 0x20, 0x49, 0x66, 0x20,
        0x69, 0x74, 0x20, 0x69, 0x73, 0x20, 0x61, 0x20, 0x68, 0x69, 0x74, 0x2c,
        0x20, 0x69, 0x74, 0x20, 0x77, 0x69, 0x6c, 0x6c, 0x20, 0x62, 0x65, 0x20,
        0x6d, 0x61, 0x72, 0x6b, 0x65, 0x64, 0x20, 0x69, 0x6e, 0x20, 0x72, 0x65,
        0x64, 0x2e, 0x20, 0x20, 0x49, 0x66, 0x20, 0x69, 0x74, 0x20, 0x69, 0x73,
        0x20, 0x61, 0x20, 0x6d, 0x69, 0x73, 0x73, 0x2c, 0x20, 0x69, 0x74, 0x20,
        0x77, 0x69, 0x6c, 0x6c, 0x20, 0x62, 0x65, 0x20, 0x6d, 0x61, 0x72, 0x6b,
        0x65, 0x64, 0x20, 0x69, 0x6e, 0x20, 0x77, 0x68, 0x69, 0x74, 0x65, 0x2e,
        0x20, 0x20, 0x59, 0x6f, 0x75, 0x72, 0x20, 0x6f, 0x70, 0x70, 0x6f, 0x6e,
        0x65, 0x6e, 0x74, 0x27, 0x73, 0x20, 0x68, 0x69, 0x74, 0x73, 0x20, 0x6f,
        0x6e, 0x20, 0x79, 0x6f, 0x75, 0x72, 0x20, 0x73, 0x68, 0x69, 0x70, 0x73,
        0x20, 0x77, 0x69, 0x6c, 0x6c, 0x20, 0x61, 0x6c, 0x73, 0x6f, 0x20, 0x62,
        0x65, 0x20, 0x6d, 0x61, 0x72, 0x6b, 0x65, 0x64, 0x20, 0x69, 0x6e, 0x20,
        0x72, 0x65, 0x64, 0x20, 0x6f, 0x6e, 0x20, 0x79, 0x6f, 0x75, 0x72, 0x20,
        0x74, 0x6f, 0x70, 0x20, 0x67, 0x72, 0x69, 0x64, 0x2e, 0x0a, 0x57, 0x68,
        0x65, 0x6e, 0x20, 0x61, 0x6c, 0x6c, 0x20, 0x74, 0x68, 0x65, 0x20, 0x63,
        0x65, 0x6c, 0x6c, 0x73, 0x20, 0x66, 0x69, 0x6c, 0x6c, 0x65, 0x64, 0x20,
        0x62, 0x79, 0x20, 0x61, 0x20, 0x73, 0x68, 0x69, 0x70, 0x20, 0x68, 0x61,
        0x76, 0x65, 0x20, 0x62, 0x65, 0x65, 0x6e, 0x20, 0x68, 0x69, 0x74, 0x2c,
        0x20, 0x6f, 0x72, 0x20, 0x69, 0x6e, 0x20, 0x6f, 0x74, 0x68, 0x65, 0x72,
        0x20, 0x77, 0x6f, 0x72, 0x64, 0x73, 0x2c, 0x20, 0x77, 0x68, 0x65, 0x6e,
        0x20, 0x61, 0x6e, 0x20, 0x65, 0x6e, 0x74, 0x69, 0x72, 0x65, 0x20, 0x73,
        0x68, 0x69, 0x70, 0x27, 0x73, 0x20, 0x6c, 0x6f, 0x63, 0x61, 0x74, 0x69,
        0x6f, 0x6e, 0x20, 0x68, 0x61, 0x73, 0x20, 0x62, 0x65, 0x65, 0x6e, 0x20,
        0x67, 0x75, 0x65, 0x73, 0x73, 0x65, 0x64, 0x2c, 0x20, 0x74, 0x68, 0x65,
        0x20, 0x73, 0x68, 0x69, 0x70, 0x20, 0x69, 0x73, 0x20, 0x73, 0x61, 0x69,
        0x64, 0x20, 0x74, 0x6f, 0x20, 0x62, 0x65, 0x20, 0x73, 0x75, 0x6e, 0x6b,
        0x2e, 0x20, 0x20, 0x41, 0x20, 0x6d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65,
        0x20, 0x61, 0x74, 0x20, 0x74, 0x68, 0x65, 0x20, 0x62, 0x6f, 0x74, 0x74,
        0x6f, 0x6d, 0x20, 0x6f, 0x66, 0x20, 0x74, 0x68, 0x65, 0x20, 0x73, 0x63,
        0x72, 0x65, 0x65, 0x6e, 0x20, 0x77, 0x69, 0x6c, 0x6c, 0x20, 0x69, 0x6e,
        0x64, 0x69, 0x63, 0x61, 0x74, 0x65, 0x20, 0x69, 0x66, 0x20, 0x61, 0x20,
        0x73, 0x68, 0x69, 0x70, 0x20, 0x68, 0x61, 0x73, 0x20, 0x62, 0x65, 0x65,
        0x6e, 0x20, 0x73, 0x75, 0x6e, 0x6b, 0x20, 0x61, 0x66, 0x74, 0x65, 0x72,
        0x20, 0x74, 0x68, 0x65, 0x20, 0x66, 0x69, 0x6e, 0x61, 0x6c, 0x20, 0x73,
        0x68, 0x6f, 0x74, 0x20, 0x6e, 0x65, 0x65, 0x64, 0x65, 0x64, 0x20, 0x74,
        0x6f, 0x20, 0x73, 0x69, 0x6e, 0x6b, 0x20, 0x74, 0x68, 0x65, 0x20, 0x73,
        0x68, 0x69, 0x70, 0x20, 0x77, 0x61, 0x73, 0x20, 0x74, 0x61, 0x6b, 0x65,
        0x6e, 0x2e, 0x0a, 0x57, 0x68, 0x65, 0x6e, 0x20, 0x69, 0x74, 0x20, 0x69,
        0x73, 0x20, 0x79, 0x6f, 0x75, 0x72, 0x20, 0x74, 0x75, 0x72, 0x6e, 0x2c,
        0x20, 0x79, 0x6f, 0x75, 0x20, 0x77, 0x69, 0x6c, 0x6c, 0x20, 0x73, 0x68,
        0x6f, 0x6f, 0x74, 0x20, 0x61, 0x74, 0x20, 0x79, 0x6f, 0x75, 0x72, 0x20,
        0x6f, 0x70, 0x70, 0x6f, 0x6e, 0x65, 0x6e, 0x74, 0x27, 0x73, 0x20, 0x73,
        0x68, 0x69, 0x70, 0x73, 0x2e, 0x20, 0x20, 0x54, 0x68, 0x65, 0x6e, 0x2c,
        0x20, 0x69, 0x74, 0x20, 0x69, 0x73, 0x20, 0x79, 0x6f, 0x75, 0x72, 0x20,
        0x6f, 0x70, 0x70, 0x6f, 0x6e, 0x65, 0x6e, 0x74, 0x27, 0x73, 0x20, 0x74,
        0x75, 0x72, 0x6e, 0x2c, 0x20, 0x61, 0x6e, 0x64, 0x20, 0x69, 0x74, 0x20,
        0x77, 0x69, 0x6c, 0x6c, 0x20, 0x64, 0x6f, 0x20, 0x74, 0x68, 0x65, 0x20,
        0x73, 0x61, 0x6d, 0x65, 0x2e, 0x20, 0x20, 0x54, 0x68, 0x69, 0x73, 0x20,
        0x70, 0x72, 0x6f, 0x63, 0x65, 0x73, 0x73, 0x20, 0x72, 0x65, 0x70, 0x65,
        0x61, 0x74, 0x73, 0x20, 0x75, 0x6e, 0x74, 0x69, 0x6c, 0x20, 0x74, 0x68,
        0x65, 0x20, 0x67, 0x61, 0x6d, 0x65, 0x20, 0x69, 0x73, 0x20, 0x6f, 0x76,
        0x65, 0x72, 0x2e, 0x0a, 0x0a, 0x33, 0x2e, 0x20, 0x57, 0x69, 0x6e, 0x6e,
        0x69, 0x6e, 0x67, 0x20, 0x74, 0x68, 0x65, 0x20, 0x67, 0x61, 0x6d, 0x65,
        0x0a, 0x54, 0x68, 0x65, 0x20, 0x67, 0x61, 0x6d, 0x65, 0x20, 0x69, 0x73,
        0x20, 0x6f, 0x76, 0x65, 0x72, 0x20, 0x61, 0x66, 0x74, 0x65, 0x72, 0x20,
        0x61, 0x20, 0x70, 0x6c, 0x61, 0x79, 0x65, 0x72, 0x27, 0x73, 0x20, 0x66,
        0x6c, 0x65, 0x65, 0x74, 0x2c, 0x20, 0x69, 0x2e, 0x65, 0x2e, 0x20, 0x61,
        0x6c, 0x6c, 0x20, 0x6f, 0x66, 0x20, 0x61, 0x20, 0x70, 0x6c, 0x61, 0x79,
        0x65, 0x72, 0x27, 0x73, 0x20, 0x73, 0x68, 0x69, 0x70, 0x73, 0x2c, 0x20,
        0x68, 0x61, 0x76, 0x65, 0x20, 0x62, 0x65, 0x65, 0x6e, 0x20, 0x73, 0x75,
        0x6e, 0x6b, 0x2e, 0x20, 0x20, 0x54, 0x68, 0x65, 0x20, 0x77, 0x69, 0x6e,
        0x6e, 0x65, 0x72, 0x20, 0x69, 0x73, 0x20, 0x77, 0x68, 0x6f, 0x65, 0x76,
        0x65, 0x72, 0x20, 0x69, 0x73, 0x20, 0x61, 0x62, 0x6c, 0x65, 0x20, 0x74,
        0x6f, 0x20, 0x73, 0x69, 0x6e, 0x6b, 0x20, 0x74, 0x68, 0x65, 0x20, 0x6f,
        0x74, 0x68, 0x65, 0x72, 0x20, 0x70, 0x6c, 0x61, 0x79, 0x65, 0x72, 0x27,
        0x73, 0x20, 0x66, 0x6c, 0x65, 0x65, 0x74, 0x20, 0x66, 0x69, 0x72, 0x73,
        0x74, 0x2e, 0x0a, 0x59, 0x6f, 0x75, 0x72, 0x20, 0x63, 0x6f, 0x6d, 0x70,
        0x75, 0x74, 0x65, 0x72, 0x20, 0x6f, 0x70, 0x70, 0x6f, 0x6e, 0x65, 0x6e,
        0x74, 0x27, 0x73, 0x20, 0x62, 0x6f, 0x61, 0x72, 0x64, 0x20, 0x77, 0x69,
        0x6c, 0x6c, 0x20, 0x61, 0x6c, 0x73, 0x6f, 0x20, 0x62, 0x65, 0x20, 0x72,
        0x65, 0x76, 0x65, 0x61, 0x6c, 0x65, 0x64, 0x20, 0x61, 0x74, 0x20, 0x74,
        0x68, 0x65, 0x20, 0x65, 0x6e, 0x64, 0x20, 0x6f, 0x66, 0x20, 0x74, 0x68,
        0x65, 0x20, 0x67, 0x61, 0x6d, 0x65, 0x2e, 0x0a, 0x0a, 0x42, 0x65, 0x73,
        0x74, 0x20, 0x6f, 0x66, 0x20, 0x6c, 0x75, 0x63, 0x6b, 0x21, 0x00};

    printf("%s\n", resources_instructions_txt);
    SDL_Delay(3000);
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
    int turn = 0, i = 0;
    bool running = true, started = false;

    srand(time(NULL)); // initialize random psuedo-random seed for random actions

    // display instructions before rendering window and beginning game
    displayInstructions();

    // initialize game elements
    initializeBoards(&p1);
    initializeBoards(&p2);
    initializeShips(&p1);
    initializeShips(&p2);
    initializeOpponent(&p2, &opGuess);
    p1.pause = false;
    p2.pause = false;

    // create game windows and renderer
    SDL_Renderer *renderer = initializeSDL(window, "Battleship", CELL_SIZE * BOARD_SIZE_X + 1, CELL_SIZE * (BOARD_SIZE_Y + 3) + 1);

    // display blank board
    render(renderer, &p1);

    // game loop
    // FPS dependent on constant game speed
    while (running)
    {
        // time exectuion of game logic and rendering
        start = clock();
        updateGame(&running, &started, &turn, &p1, &p2, &opGuess);
        render(renderer, &p1); // render player's grid

        // pause updating game state for ~1s
        if (p1.pause)
            i++;
        if (i >= 99)
        {
            i = 0;
            p1.pause = false;
        }

        end = clock();

        sleepTime = SKIP_TICKS - ((double)(end - start) / CLOCKS_PER_SEC); // calculate time to sleep between rendering each frame
        (sleepTime >= 0) ? SDL_Delay(sleepTime) : printf("Running %lfs behind!\n", -1 * sleepTime);
    }

    // display end of game msgs
    SDL_Delay(1000);
    p1.msg = 8; // game over msg
    render(renderer, &p1);
    SDL_Delay(1000);
    p1.msg = (checkGameOver(&p2)) ? 16 : 17; // win or lose msg
    render(renderer, &p1);
    SDL_Delay(3000);

    // reveal computer player's board at end of game
    p2.msg = 13; // opponent's board msg
    render(renderer, &p2);
    SDL_Delay(5000);

    teardown(renderer, window);

    return 0;
}
