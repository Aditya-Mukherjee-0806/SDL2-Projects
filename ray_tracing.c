#include <SDL2/SDL.h>

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define PI 3.1415926
#define RAYS_SIZE 1000
#define THRESHOLD 1
#define RGBCOLOR_BG_LIT 192, 192, 192
#define RGBCOLOR_BG_DARK 64, 64, 64
#define RGBCOLOR_CIRCLE_BRIGHT 255, 255, 255
#define RGBCOLOR_CIRCLE_OPAQUE 0, 0, 0

typedef struct
{
    double x, y, r;
} CIRCLE;

int main()
{
    void FillCircle(SDL_Surface *, CIRCLE *, int color);
    void CastRays(SDL_Surface *, CIRCLE *, CIRCLE *);

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window *window = SDL_CreateWindow("Ray Tracing", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Surface *surface = SDL_GetWindowSurface(window);

    CIRCLE circle_bright = {200, 250, 75};
    CIRCLE circle_opaque = {800, 360, 125};

    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, RGBCOLOR_BG_DARK));
    FillCircle(surface, &circle_bright, SDL_MapRGB(surface->format, RGBCOLOR_CIRCLE_BRIGHT));
    FillCircle(surface, &circle_opaque, SDL_MapRGB(surface->format, RGBCOLOR_CIRCLE_OPAQUE));
    CastRays(surface, &circle_bright, &circle_opaque);
    SDL_UpdateWindowSurface(window);

    int running = 1;
    while (running)
    {
        SDL_Event event;
        int updated = 0;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                running = 0;
            else if (event.type == SDL_MOUSEMOTION && event.motion.state == SDL_PRESSED)
            {
                circle_bright.x = event.motion.x;
                circle_bright.y = event.motion.y;
                updated = 1;
            }
        }
        if(updated)
        {
            SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, RGBCOLOR_BG_DARK));
            FillCircle(surface, &circle_bright, SDL_MapRGB(surface->format, RGBCOLOR_CIRCLE_BRIGHT));
            FillCircle(surface, &circle_opaque, SDL_MapRGB(surface->format, RGBCOLOR_CIRCLE_OPAQUE));
            CastRays(surface, &circle_bright, &circle_opaque);
            SDL_UpdateWindowSurface(window);
        }
        SDL_Delay(10);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

void FillCircle(SDL_Surface *surface, CIRCLE *circle, int color)
{
    SDL_Rect pixel_rect = {circle->x, circle->y, 1, 1};
    for (int x = circle->x - circle->r; x < circle->x + circle->r; x++)
    {
        for (int y = circle->y - circle->r; y < circle->y + circle->r; y++)
        {
            pixel_rect.x = x;
            pixel_rect.y = y;
            double d = SDL_sqrt(SDL_pow(x - circle->x, 2) + SDL_pow(y - circle->y, 2));
            if (d <= circle->r)
                SDL_FillRect(surface, &pixel_rect, color);
        }
    }
}

void CastRays(SDL_Surface *surface, CIRCLE *circle_bright, CIRCLE *circle_opaque)
{
    double clamp(double, double, double);
    int PixelInsideCircle(SDL_Rect, CIRCLE);

    SDL_Rect pixel_rect = {0, 0, 1, 1};
    for (int i = 0; i < RAYS_SIZE; i++)
    {
        double θ = (double)i / RAYS_SIZE * 2 * PI;
        double cos_θ = SDL_cos(θ), sin_θ = SDL_sin(θ);
        for (double r = 0; r < WINDOW_WIDTH; r += THRESHOLD)
        {
            int x = r * cos_θ + circle_bright->x;
            int y = r * sin_θ + circle_bright->y;

            if ((x < 0 && y < 0) || (x >= WINDOW_WIDTH && y >= WINDOW_HEIGHT))
                break;

            x = clamp(x, 0, WINDOW_WIDTH - 1);
            y = clamp(y, 0, WINDOW_HEIGHT - 1);

            pixel_rect.x = x;
            pixel_rect.y = y;

            if (PixelInsideCircle(pixel_rect, *circle_bright))
                continue;
            if (PixelInsideCircle(pixel_rect, *circle_opaque))
                break;
            SDL_FillRect(surface, &pixel_rect, SDL_MapRGB(surface->format, RGBCOLOR_BG_LIT));
        }
    }
}

double clamp(double value, double min, double max)
{
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}

int PixelInsideCircle(SDL_Rect pixel, CIRCLE circle)
{
    double d = SDL_sqrt(SDL_pow(pixel.x - circle.x, 2) + SDL_pow(pixel.y - circle.y, 2));
    return d <= circle.r;
}