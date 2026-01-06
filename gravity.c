#include <SDL2/SDL.h>

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#define RGB_WHITE 255, 255, 255
#define RGB_RED 255, 0, 0
#define RGB_GREEN 0, 255, 0
#define RGB_BLUE 0, 0, 255

#define PIXEL_TO_METER_RATIO 100
#define DELTA_TIME_MS 10
#define STD_GRAVITY 9.80665

const double g = STD_GRAVITY * PIXEL_TO_METER_RATIO / 1000000;

typedef struct
{
    double mass;
    double vel_x, vel_y;
    double acc_x, acc_y;
} PHYSICS_OBJ;

typedef struct
{
    double x, y, r;
    PHYSICS_OBJ *physics_obj;
    Uint32 color;
} CIRCLE;

void FillCircle(SDL_Surface * surface, CIRCLE * circle, Uint32 color);
void UpdateCirclePosition(SDL_Surface *surface, CIRCLE *circle);
double clamp(double val, double min, double max);

int main()
{
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window *window = SDL_CreateWindow("Bouncy Ball", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Surface *surface = SDL_GetWindowSurface(window);

    PHYSICS_OBJ physics_obj = {100, 0, 0, 0, g};
    CIRCLE ball = {WINDOW_WIDTH / 2, 100, 50, &physics_obj, SDL_MapRGB(surface->format, RGB_GREEN)};

    Uint8 running = 1;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                running = 0;
        }
        SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, RGB_WHITE));
        UpdateCirclePosition(surface, &ball);
        if(ball.y + ball.r == WINDOW_HEIGHT)
            running = 0;
        SDL_UpdateWindowSurface(window);
        SDL_Delay(DELTA_TIME_MS);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

void FillCircle(SDL_Surface *surface, CIRCLE *circle, Uint32 color)
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

void UpdateCirclePosition(SDL_Surface *surface, CIRCLE *circle)
{
    if(circle->physics_obj->vel_x)
    {
        double disp_x = circle->physics_obj->vel_x * DELTA_TIME_MS + 0.5 * circle->physics_obj->acc_x * SDL_pow(DELTA_TIME_MS, 2);
        circle->x += disp_x;
        circle->x = clamp(circle->x, 0, WINDOW_WIDTH - circle->r);
    }
    if (circle->physics_obj->vel_y)
    {
        double disp_y = circle->physics_obj->vel_y * DELTA_TIME_MS + 0.5 * circle->physics_obj->acc_y * SDL_pow(DELTA_TIME_MS, 2);
        circle->y += disp_y;
        circle->y = clamp(circle->y, 0, WINDOW_HEIGHT - circle->r);
    }

    circle->physics_obj->vel_x += circle->physics_obj->acc_x * DELTA_TIME_MS;
    circle->physics_obj->vel_y += circle->physics_obj->acc_y * DELTA_TIME_MS;

    FillCircle(surface, circle, circle->color);
}

double clamp(double val, double min, double max)
{
    if(val < min)
        val = min;
    else if(val > max)
        val = max;
    return val;
}
