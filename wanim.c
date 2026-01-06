#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#define WINDOW_WIDTH 960
#define WINDOW_HEIGHT 540

int main()
{
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window *window = SDL_CreateWindow("Animation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Surface *surface = SDL_GetWindowSurface(window);

    SDL_Rect pixel;
    pixel.w = pixel.h = 1;

    const int COLOR_WHITE = SDL_MapRGB(surface->format, 255, 255, 255);

    for (int i = 0; i < WINDOW_HEIGHT; i += pixel.h)
    {
        for (int j = 0; j < WINDOW_WIDTH; j += pixel.w)
        {
            pixel.y = i;
            pixel.x = j;
            SDL_FillRect(surface, &pixel, COLOR_WHITE);
            SDL_UpdateWindowSurface(window);
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}