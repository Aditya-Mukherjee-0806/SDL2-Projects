#include <stdio.h>
#include <SDL2/SDL.h>

#define WINDOW_WIDTH 960
#define WINDOW_HEIGHT 540

int main()
{
    printf("Creating a test window using SDL2\n");

    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window *window = SDL_CreateWindow("test window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Surface *surface = SDL_GetWindowSurface(window);

    SDL_Rect *top_left_rect = (SDL_Rect *)malloc(sizeof(SDL_Rect));
    top_left_rect->x = 0;
    top_left_rect->y = 0;
    top_left_rect->w = WINDOW_WIDTH / 2;
    top_left_rect->h = WINDOW_HEIGHT / 2;
    SDL_FillRect(surface, top_left_rect, SDL_MapRGB(surface->format, 0, 0, 255));

    SDL_Rect *top_right_rect = (SDL_Rect *)malloc(sizeof(SDL_Rect));
    top_right_rect->x = WINDOW_WIDTH / 2;
    top_right_rect->y = 0;
    top_right_rect->w = WINDOW_WIDTH / 2;
    top_right_rect->h = WINDOW_HEIGHT / 2;
    SDL_FillRect(surface, top_right_rect, SDL_MapRGB(surface->format, 0, 255, 0));

    SDL_Rect *bottom_left_rect = (SDL_Rect *)malloc(sizeof(SDL_Rect));
    bottom_left_rect->x = 0;
    bottom_left_rect->y = WINDOW_HEIGHT / 2;
    bottom_left_rect->w = WINDOW_WIDTH / 2;
    bottom_left_rect->h = WINDOW_HEIGHT / 2;
    SDL_FillRect(surface, bottom_left_rect, SDL_MapRGB(surface->format, 0, 255, 255));

    SDL_Rect *bottom_right_rect = (SDL_Rect *)malloc(sizeof(SDL_Rect));
    bottom_right_rect->x = WINDOW_WIDTH / 2;
    bottom_right_rect->y = WINDOW_HEIGHT / 2;
    bottom_right_rect->w = WINDOW_WIDTH / 2;
    bottom_right_rect->h = WINDOW_HEIGHT / 2;
    SDL_FillRect(surface, bottom_right_rect, SDL_MapRGB(surface->format, 255, 0, 0));

    SDL_UpdateWindowSurface(window);

    while (1)
    {
        if (getchar())
            break;
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}