#include <SDL2/SDL.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH 1280
#define HEIGHT 720
#define FRAME_RATE 100
#define POINT_SIZE 5
#define NUM_POINTS NUM_COLORS

#define COLOR_RED 255, 0, 0
#define COLOR_GREEN 0, 255, 0
#define COLOR_BLUE 0, 0, 255
#define COLOR_YELLOW 255, 255, 0
#define COLOR_CYAN 0, 255, 255
#define COLOR_MAGENTA 255, 0, 255

typedef struct
{
    short x, y;
    Uint32 color;
} POINT;

enum DIR
{
    LEFT,
    RIGHT,
    UP,
    DOWN,
    DIR_SIZE
};

enum COLOR
{
    RED,
    BLUE,
    GREEN,
    YELLOW,
    CYAN,
    MAGENTA,
    NUM_COLORS
};

void initPoints(POINT points[], int len, SDL_Surface *surface);
void randomStepPoints(POINT points[], int len);
void clampPointPosition(POINT *point);
void FillPoints(POINT points[], int len, SDL_Surface *surface);

int main()
{
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window *window = SDL_CreateWindow("Random Walk", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Surface *surface = SDL_GetWindowSurface(window);
    srand(time(NULL));
    POINT points[NUM_POINTS];
    initPoints(points, NUM_POINTS, surface);

    Uint8 running = 1;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                running = 0;
                break;
            }
        }
        randomStepPoints(points, NUM_POINTS);
        FillPoints(points, NUM_POINTS, surface);
        SDL_UpdateWindowSurface(window);
        SDL_Delay(1000 / FRAME_RATE);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

void initPoints(POINT points[], int len, SDL_Surface *surface)
{
    for (int i = 0; i < len; i++)
    {
        points[i].x = (WIDTH - POINT_SIZE) / 2;
        points[i].y = (HEIGHT - POINT_SIZE) / 2;
        Uint8 color_index = i % NUM_COLORS;
        switch (color_index)
        {
        case RED:
            points[i].color = SDL_MapRGB(surface->format, COLOR_RED);
            break;
        case GREEN:
            points[i].color = SDL_MapRGB(surface->format, COLOR_GREEN);
            break;
        case BLUE:
            points[i].color = SDL_MapRGB(surface->format, COLOR_BLUE);
            break;
        case YELLOW:
            points[i].color = SDL_MapRGB(surface->format, COLOR_YELLOW);
            break;
        case CYAN:
            points[i].color = SDL_MapRGB(surface->format, COLOR_CYAN);
            break;
        case MAGENTA:
            points[i].color = SDL_MapRGB(surface->format, COLOR_MAGENTA);
            break;
        }
    }
}

void randomStepPoints(POINT points[], int len)
{
    for (int i = 0; i < len; i++)
    {
        Uint8 dir = rand() % DIR_SIZE;
        switch (dir)
        {
        case LEFT:
            points[i].x -= POINT_SIZE;
            break;
        case RIGHT:
            points[i].x += POINT_SIZE;
            break;
        case UP:
            points[i].y -= POINT_SIZE;
            break;
        case DOWN:
            points[i].y += POINT_SIZE;
        }
        clampPointPosition(points + i);
    }
}

void clampPointPosition(POINT *point)
{
    if (point->x < 0)
        point->x = 0;
    else if (point->x > WIDTH - POINT_SIZE)
        point->x = WIDTH - POINT_SIZE;
    if (point->y < 0)
        point->y = 0;
    else if (point->y > HEIGHT - POINT_SIZE)
        point->y = HEIGHT - POINT_SIZE;
}

void FillPoints(POINT points[], int len, SDL_Surface *surface)
{
    for (int i = 0; i < len; i++)
    {
        SDL_Rect point_rect = {points[i].x, points[i].y, POINT_SIZE, POINT_SIZE};
        SDL_FillRect(surface, &point_rect, points[i].color);
    }
}