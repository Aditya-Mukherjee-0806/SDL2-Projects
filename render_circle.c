#include <SDL2/SDL.h>

#define WIDTH 1280
#define HEIGHT 720

typedef struct
{
    double x, y, r;
} CIRCLE;

int main()
{
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window *window = SDL_CreateWindow("Render Circle", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    CIRCLE c = {100, 100, 100};
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, c.r * 2, c.r * 2, 32, SDL_PIXELFORMAT_RGBA8888);
    SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);
    SDL_FillRect(surface, NULL, SDL_MapRGBA(surface->format, 0, 0, 0, 0));
    for (int i = 0; i < c.r + c.r; i++)
    {
        for (int j = 0; j < c.r + c.r; j++)
        {
            double x_dist = i - c.x;
            double y_dist = j - c.y;
            double dist_sq = x_dist * x_dist + y_dist * y_dist;
            if (dist_sq <= c.r * c.r)
                SDL_FillRect(surface, &(SDL_Rect){i, j, 1, 1}, SDL_MapRGBA(surface->format, 255, 255, 255, 255));
        }
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_FreeSurface(surface);
    SDL_SetRenderDrawColor(renderer, 32, 32, 32, 255);
    SDL_RenderClear(renderer);
    SDL_Rect des_rect = {WIDTH / 2, HEIGHT / 2, c.r * 2, c.r * 2};
    SDL_RenderCopy(renderer, texture, NULL, &des_rect);
    SDL_RenderPresent(renderer);
    SDL_Delay(5000);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}