#ifndef __RENDERING_H__
#define __RENDERING_H__

#include <SDL2/SDL.h>
#include "engine.h"

/* Render Game
**********************************************************************
*********************************************************************/

/* Draw Objects
*********************************************************************/
void drawBackground(SDL_Renderer *renderer, int r, int g, int b, int a);
void drawGrids(SDL_Renderer *renderer, int r, int g, int b, int a);
void drawPlayerShips(SDL_Renderer *renderer, player *p);
/********************************************************************/

void render(SDL_Renderer *renderer, player *p);

/*********************************************************************
*********************************************************************/

#endif