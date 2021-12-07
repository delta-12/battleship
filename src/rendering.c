#include "rendering.h"

/* Render Game
**********************************************************************
*********************************************************************/

/* Draw Objects
*********************************************************************/
// draw solid background on window
void drawBackground(SDL_Renderer *renderer, int r, int g, int b, int a)
{
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderClear(renderer);
}

// draw lines
void drawGrids(SDL_Renderer *renderer, int r, int g, int b, int a)
{
    SDL_SetRenderDrawColor(renderer, r, g, b, a);

    // draw 2 10 x 10 grids
    for (int i = 1; i < 12; i++)
    {
        // top grid
        SDL_RenderDrawLine(renderer, CELL_SIZE + 1, i * CELL_SIZE + 1, CELL_SIZE * 11 + 1, i * CELL_SIZE + 1); // horizontal lines
        SDL_RenderDrawLine(renderer, i * CELL_SIZE + 1, CELL_SIZE + 1, i * CELL_SIZE + 1, CELL_SIZE * 11 + 1); // vertical lines

        // bottom grid
        SDL_RenderDrawLine(renderer, CELL_SIZE + 1, (i + 11) * CELL_SIZE + 1, CELL_SIZE * 11 + 1, (i + 11) * CELL_SIZE + 1);
        SDL_RenderDrawLine(renderer, i * CELL_SIZE + 1, CELL_SIZE * 12 + 1, i * CELL_SIZE + 1, CELL_SIZE * 22 + 1);
    }

    // draw vertical dividing line
    SDL_RenderDrawLine(renderer, 12 * CELL_SIZE + 1, 0, 12 * CELL_SIZE + 1, CELL_SIZE * BOARD_SIZE_Y + 1);

    // draw selection box
    SDL_RenderDrawLine(renderer, 13 * CELL_SIZE + 1, CELL_SIZE + 1, 18 * CELL_SIZE + 1, CELL_SIZE + 1);         // top
    SDL_RenderDrawLine(renderer, 13 * CELL_SIZE + 1, 6 * CELL_SIZE + 1, 18 * CELL_SIZE + 1, 6 * CELL_SIZE + 1); // bottom
    SDL_RenderDrawLine(renderer, 13 * CELL_SIZE + 1, CELL_SIZE + 1, 13 * CELL_SIZE + 1, 6 * CELL_SIZE + 1);     // left
    SDL_RenderDrawLine(renderer, 18 * CELL_SIZE + 1, CELL_SIZE + 1, 18 * CELL_SIZE + 1, 6 * CELL_SIZE + 1);     // right
}

void drawPlayerShips(SDL_Renderer *renderer, player *p)
{
    int cellVal;

    for (int i = 0; i < BOARD_SIZE_X; i++)
    {
        for (int j = 0; j < BOARD_SIZE_Y; j++)
        {
            cellVal = p->grid[i][j];
            switch (cellVal)
            {
            case 1:
                SDL_SetRenderDrawColor(renderer, 22, 198, 12, 255); // player's ship- green
                break;
            case 2:
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // miss- white
                break;
            case 3:
                SDL_SetRenderDrawColor(renderer, 197, 15, 31, 255); // hit- red
                break;
            default:
                continue;
            }
            SDL_Rect rect = {i * CELL_SIZE + 2, j * CELL_SIZE + 2, CELL_SIZE - 1, CELL_SIZE - 1};
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}
/********************************************************************/

// display objects in window
void render(SDL_Renderer *renderer, player *p)
{
    SDL_Color gridBackground = {22, 22, 22, 255}; // Barely Black
    SDL_Color gridLineColor = {44, 44, 44, 255};  // Dark grey

    drawBackground(renderer, gridBackground.r, gridBackground.g, gridBackground.b, gridBackground.a);
    drawGrids(renderer, gridLineColor.r, gridLineColor.g, gridLineColor.b, gridLineColor.a);
    drawPlayerShips(renderer, p);
    SDL_RenderPresent(renderer);
    SDL_RenderClear(renderer);
}

/*********************************************************************
*********************************************************************/