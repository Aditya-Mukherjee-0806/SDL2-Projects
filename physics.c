#include <SDL2/SDL.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH 1280
#define HEIGHT 720
#define FRAMES_PER_SEC 60
#define DEFAULT_CAPACITY 5
#define MIN_RADIUS 10
#define MAX_RADIUS 100
#define PIXELS_PER_METER 100
#define DENSITY 100000

#define RGB_RED 255, 0, 0
#define RGB_GREEN 0, 255, 0
#define RGB_BLUE 0, 0, 255
#define RGB_YELLOW 255, 255, 0
#define RGB_CYAN 0, 255, 255
#define RGB_MAGENTA 255, 0, 255

typedef struct
{
    double x, y;
} VECTOR_2D;

typedef struct
{
    double mass;
    VECTOR_2D pos, vel;
} PHYS_OBJ;

typedef struct
{
    Uint32 color;
    double radius;
    PHYS_OBJ node;
} CIRCLE;

const double π = 3.1415926;
const double G = 6.6743E-11;
const double dt = 1000.0 / FRAMES_PER_SEC;
const CIRCLE NULL_CIRCLE = {
    .color = 0,
    .radius = -1,
    .node = {
        .mass = 0,
        .pos = {-1, -1},
        .vel = {-1, -1},
    },
};

SDL_Surface *surface;
CIRCLE *circles;
int cap, size = 0;

void createCircle(Uint32 color, double radius, double mass, VECTOR_2D pos, VECTOR_2D vel);
void runSimulation();
void sanitiseCircles();
void simulateForces();
void simulateGravitationalForce();
void updatePositions();
void FillCircles();
void FillCircle(CIRCLE circle);
int isNullCircle(CIRCLE circle);

int main()
{
    srand(time(NULL));
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window *window = SDL_CreateWindow("Physics Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    surface = SDL_GetWindowSurface(window);

    circles = (CIRCLE *)calloc(DEFAULT_CAPACITY, sizeof(CIRCLE));
    cap = DEFAULT_CAPACITY;

    const Uint32 colors[] = {
        SDL_MapRGB(surface->format, RGB_RED),
        SDL_MapRGB(surface->format, RGB_GREEN),
        SDL_MapRGB(surface->format, RGB_BLUE),
        SDL_MapRGB(surface->format, RGB_YELLOW),
        SDL_MapRGB(surface->format, RGB_CYAN),
        SDL_MapRGB(surface->format, RGB_MAGENTA),
    };
    const int num_colors = sizeof(colors) / sizeof(colors[0]);

    for (int i = 0; i < cap; i++)
    {
        int radius = rand() % (MAX_RADIUS - MIN_RADIUS) + MIN_RADIUS;
        Uint32 color = colors[i % num_colors];
        VECTOR_2D pos = {rand() % WIDTH, rand() % HEIGHT};
        createCircle(color, radius, π * radius * radius * DENSITY, pos, (VECTOR_2D){0, 0});
    }

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
        if (!size)
            break;
        SDL_FillRect(surface, NULL, 0);
        runSimulation();
        SDL_UpdateWindowSurface(window);
        SDL_Delay(dt);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

void createCircle(Uint32 color, double radius, double mass, VECTOR_2D pos, VECTOR_2D vel)
{
    PHYS_OBJ node;
    node.mass = mass;
    node.pos = pos;
    node.vel = vel;

    CIRCLE circle;
    circle.color = color;
    circle.radius = radius;
    circle.node = node;

    if (size >= cap)
    {
        CIRCLE *temp = (CIRCLE *)realloc(circles, cap * 2);
        if (temp)
        {
            circles = temp;
            cap *= 2;
        }
        else
            printf("REALLOCATION FAILED.");
    }
    circles[size++] = circle;
}

void runSimulation()
{
    sanitiseCircles();
    simulateForces();
    updatePositions();
    FillCircles();
}

void sanitiseCircles()
{
    for (int i = 0; i < size; i++)
    {
        if (!isNullCircle(circles[i]))
            continue;
        for (int j = i + 1; j < size; j++)
            circles[j - 1] = circles[j];
        size--;
        i--;
    }
}

void simulateForces()
{
    simulateGravitationalForce();
}

void simulateGravitationalForce()
{
    for (int i = 0; i < size - 1; i++)
    {
        if (isNullCircle(circles[i]))
            continue;
        PHYS_OBJ curr_node = circles[i].node;
        for (int j = i + 1; j < size; j++)
        {
            if (isNullCircle(circles[j]))
                continue;
            PHYS_OBJ target_node = circles[j].node;
            double dist = SDL_sqrt(SDL_pow(curr_node.pos.x - target_node.pos.x, 2) + SDL_pow(curr_node.pos.y - target_node.pos.y, 2));
            if (dist + circles[i].radius <= circles[j].radius)
            {
                circles[j].node.mass += circles[i].node.mass;
                circles[i] = NULL_CIRCLE;
                continue;
            }
            else if (dist + circles[j].radius <= circles[i].radius)
            {
                circles[i].node.mass += circles[j].node.mass;
                circles[j] = NULL_CIRCLE;
                continue;
            }
            double force_magnitude = G * curr_node.mass * target_node.mass / SDL_pow(dist, 3);
            VECTOR_2D force;
            force.x = force_magnitude * (target_node.pos.x - curr_node.pos.x);
            force.y = force_magnitude * (target_node.pos.y - curr_node.pos.y);
            circles[i].node.vel.x += force.x / curr_node.mass * dt;
            circles[i].node.vel.y += force.y / curr_node.mass * dt;
            circles[j].node.vel.x -= force.x / target_node.mass * dt;
            circles[j].node.vel.y -= force.y / target_node.mass * dt;
        }
    }
}

void updatePositions()
{
    for (int i = 0; i < size; i++)
    {
        if (circles[i].node.vel.x)
            circles[i].node.pos.x += circles[i].node.vel.x * dt;
        if (circles[i].node.vel.y)
            circles[i].node.pos.y += circles[i].node.vel.y * dt;

        if (circles[i].node.pos.x + circles[i].radius <= 0)
            circles[i] = NULL_CIRCLE;
        else if (circles[i].node.pos.y + circles[i].radius <= 0)
            circles[i] = NULL_CIRCLE;
        else if (circles[i].node.pos.x - circles[i].radius >= WIDTH)
            circles[i] = NULL_CIRCLE;
        else if (circles[i].node.pos.y - circles[i].radius >= HEIGHT)
            circles[i] = NULL_CIRCLE;
    }
}

void FillCircles()
{
    for (int i = 0; i < size; i++)
        FillCircle(circles[i]);
}

void FillCircle(CIRCLE circle)
{
    int x = circle.node.pos.x;
    int y = circle.node.pos.y;
    int r = circle.radius;
    for (int i = x - r; i < x + r; i++)
    {
        if (i < 0 || i >= WIDTH - 1)
            continue;
        for (int j = y - r; j < y + r; j++)
        {
            if (j < 0 || j >= HEIGHT - 1)
                continue;
            double dist_sqr = SDL_pow(i - x, 2) + SDL_pow(j - y, 2);
            if (dist_sqr <= r * r)
            {
                SDL_Rect pixel = {i, j, 1, 1};
                SDL_FillRect(surface, &pixel, circle.color);
            }
        }
    }
}

int isNullCircle(CIRCLE circle)
{
    return circle.color == NULL_CIRCLE.color &&
           circle.radius == NULL_CIRCLE.radius &&
           circle.node.mass == NULL_CIRCLE.node.mass &&
           circle.node.pos.x == NULL_CIRCLE.node.pos.x &&
           circle.node.pos.y == NULL_CIRCLE.node.pos.y &&
           circle.node.vel.x == NULL_CIRCLE.node.vel.x &&
           circle.node.vel.y == NULL_CIRCLE.node.vel.y;
}