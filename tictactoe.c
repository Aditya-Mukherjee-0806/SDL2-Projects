#include <SDL2/SDL.h>

#define WIDTH 1280
#define HEIGHT 720

int main()
{
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window *window = SDL_CreateWindow("Tic Tac Toe", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}