#include <SDL2/SDL.h>
#define WIDTH 1280
#define HEIGHT 720
int main()
{
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window *window = SDL_CreateWindow("Draggable Rectangle", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Surface *surface = SDL_GetWindowSurface(window);

    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 255, 255, 255));

    Uint8 running = 1, is_start = 1;
    int start_x, start_y, curr_x, curr_y;
    while(running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if(event.type == SDL_QUIT)
                running = 0;
            else if(event.type == SDL_MOUSEMOTION && event.motion.state == SDL_PRESSED)
            {
                curr_x = event.motion.x;
                curr_y = event.motion.y;
                if (is_start)
                {
                    start_x = curr_x;
                    start_y = curr_y;
                    is_start = 0;
                }
            }
            else if(event.motion.state == SDL_RELEASED)
                is_start = 1;
        }
        
        SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 255, 255, 255));

        if (!is_start)
        {
            SDL_Rect rect = {start_x, start_y, curr_x - start_x, curr_y - start_y};
            if(rect.w < 0)
            {
                rect.x += rect.w;
                rect.w = -rect.w;
            }
            if(rect.h < 0)
            {
                rect.y += rect.h;
                rect.h = -rect.h;
            }
            SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, 97, 175, 239));
        }            
        
        SDL_UpdateWindowSurface(window);
        SDL_Delay(10);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}