#include <SDL2/SDL.h>

int main()
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Texture Rendering", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, 0);
    SDL_Surface *surface = SDL_LoadBMP("sample_medium.bmp");
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    SDL_Rect src = {500, 400, 400, 300};
    SDL_Rect dest = {0, 0, src.w, src.h};
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, &src, &dest);
    SDL_RenderPresent(renderer);
    SDL_Delay(3000);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}