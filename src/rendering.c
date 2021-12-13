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

    // message box
    SDL_RenderDrawLine(renderer, 0, CELL_SIZE * BOARD_SIZE_Y + 1, CELL_SIZE * BOARD_SIZE_X + 1, CELL_SIZE * BOARD_SIZE_Y + 1);
}

// color in non-zero grid values corresponding to status
void drawPlayerShips(SDL_Renderer *renderer, player *p)
{
    for (int i = 0; i < BOARD_SIZE_X; i++)
    {
        for (int j = 0; j < BOARD_SIZE_Y; j++)
        {
            switch (p->grid[i][j])
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

// create texture of each ship's number to display on ship
void createNumberTextures(SDL_Renderer *renderer, player *p)
{
    TTF_Font *font = TTF_OpenFont("FreeMonoBold.ttf", 25); // select font from file
    SDL_Color color = {255, 255, 255};                     // color text white
    SDL_Texture *texture;                                  // texture for rendering text

    // surfaces for each ship's text
    SDL_Surface *surfaces[5] = {
        TTF_RenderText_Solid(font, "1", color), // ship selection numbers
        TTF_RenderText_Solid(font, "2", color),
        TTF_RenderText_Solid(font, "3", color),
        TTF_RenderText_Solid(font, "4", color),
        TTF_RenderText_Solid(font, "5", color)};

    for (int i = 0; i < NSHIPS; i++)
    {
        texture = SDL_CreateTextureFromSurface(renderer, surfaces[i]);    // create texture from surface w/ number text
        SDL_RenderCopy(renderer, texture, NULL, &p->ships[i].numberRect); // copy texture to rendering target in ship's numberRect
        SDL_FreeSurface(surfaces[i]);                                     // free surface's memory after texture created from surface is copied to renderer
    }

    // free texture memory, close font
    SDL_DestroyTexture(texture);
    TTF_CloseFont(font);
}

// displays player's messages at bottom of window
void displayMessage(SDL_Renderer *renderer, player *p)
{
    TTF_Font *font = TTF_OpenFont("FreeMonoBold.ttf", 25);                     // select font from file
    SDL_Color color = {255, 255, 255};                                         // color text white
    SDL_Surface *surface = TTF_RenderText_Solid(font, message(p->msg), color); // surface for message text
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);    // texture for rendering text

    // determine size of rectangle
    const char *msg = message(p->msg);
    int len = (p->msg == 5 || p->msg == 6) ? 5 * 13 : (sizeof(msg) / sizeof(msg[0]) - 1) * 30;

    // area of rendering to display message
    // centered using msg len
    SDL_Rect msgRect = {((CELL_SIZE * BOARD_SIZE_X + 1) / 2) - len / 2, CELL_SIZE * (BOARD_SIZE_Y + 1) + 1, len, 25};

    // copy texture to message box area in rendering
    SDL_RenderCopy(renderer, texture, NULL, &msgRect);

    // free texture and surface memory, close font
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
    TTF_CloseFont(font);
}
/********************************************************************/

// display objects in window
void render(SDL_Renderer *renderer, player *p)
{
    SDL_Color gridBackground = {22, 22, 22, 255}; // almost black
    SDL_Color gridLineColor = {44, 44, 44, 255};  // dark grey

    // draw objects
    drawBackground(renderer, gridBackground.r, gridBackground.g, gridBackground.b, gridBackground.a);
    drawGrids(renderer, gridLineColor.r, gridLineColor.g, gridLineColor.b, gridLineColor.a);
    drawPlayerShips(renderer, p);
    createNumberTextures(renderer, p);
    displayMessage(renderer, p);

    // render objects
    SDL_RenderPresent(renderer);
    SDL_RenderClear(renderer);
}

/*********************************************************************
*********************************************************************/