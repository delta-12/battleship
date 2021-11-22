#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <SDL2/SDL.h>

SDL_Renderer *initializeSDL(SDL_Window *window, const char *title, int width, int height);

void teardown(SDL_Renderer *renderer, SDL_Window *window);

SDL_Event getKeypress(int fps);

void drawBackground(SDL_Renderer *renderer, int r, int g, int b, int a);

#endif