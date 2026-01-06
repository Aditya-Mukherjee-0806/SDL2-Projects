#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH 1600
#define HEIGHT 900
#define FRAMES_PER_SEC 60
#define DEFAULT_CAPACITY 25
#define PIXELS_PER_METER 1500
#define MIN_RADIUS 15
#define MAX_RADIUS 30
#define DENSITY 500
#define LOG_INTERVAL 0.2

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
    Uint8 alive;
    Uint16 id;
    Uint32 color;
    double radius;
    PHYS_OBJ node;
} CIRCLE;

const double π = 3.141592653589793;
const double G = 6.6743E-11 * PIXELS_PER_METER * PIXELS_PER_METER * PIXELS_PER_METER;
const double dt = 1.0 / FRAMES_PER_SEC;

FILE *log_file;
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
void displayInfo();
void printCircleInfo(CIRCLE circle);

int main()
{
    srand(time(NULL));
    log_file = fopen("phys_sim_log.txt", "w");
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

    // for (int i = 0; i < cap; i++)
    // {
    //     int radius = rand() % (MAX_RADIUS - MIN_RADIUS) + MIN_RADIUS;
    //     Uint32 color = colors[i % num_colors];
    //     VECTOR_2D pos = {rand() % WIDTH, rand() % HEIGHT};
    //     VECTOR_2D vel = {
    //         (double)rand() / RAND_MAX * (rand() % 2 ? 1 : -1),
    //         (double)rand() / RAND_MAX * (rand() % 2 ? 1 : -1),
    //     };
    //     createCircle(color, radius, π * radius * radius * DENSITY, pos, vel);
    // }

    // Central massive body (like the Sun)
    VECTOR_2D pos1 = {WIDTH / 2.0, HEIGHT / 2.0};
    VECTOR_2D vel1 = {0, 0};
    double radius1 = 40;
    double mass1 = 100000000;
    createCircle(SDL_MapRGB(surface->format, RGB_YELLOW), radius1, mass1, pos1, vel1);

    // Smaller orbiting body (like a planet)
    double distance = 300; // distance from center
    VECTOR_2D pos2 = {pos1.x + distance, pos1.y};
    double radius2 = 20; // smaller radius
    double mass2 = 100000;

    // Circular orbit velocity perpendicular to radius
    double v = SDL_sqrt(G * mass1 / distance);
    VECTOR_2D vel2 = {0, -v}; // moving upwards for clockwise orbit
    createCircle(SDL_MapRGB(surface->format, RGB_CYAN), radius2, mass2, pos2, vel2);

    Uint8 running = 1;
    double time_passed = 0;
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
        if (size == 0 || size == 1)
        {
            displayInfo();
            break;
        }
        SDL_FillRect(surface, NULL, 0);
        runSimulation();
        SDL_UpdateWindowSurface(window);
        if (time_passed >= LOG_INTERVAL)
        {
            time_passed = 0;
            displayInfo();
        }
        time_passed += dt;
        SDL_Delay(dt * 1000);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    fclose(log_file);
    free(circles);
    return 0;
}

void displayInfo()
{
    static int log_count = 1;
    printf("Log Entry: #%d\n", log_count);
    fprintf(log_file, "ENTRY: #%d\n", log_count);
    for (int i = 0; i < size; i++)
    {
        printf("Circle %d:\n", circles[i].id);
        fprintf(log_file, "Circle %d:\n", circles[i].id);
        printCircleInfo(circles[i]);
    }
    log_count++;
}

void printCircleInfo(CIRCLE circle)
{
    if (!circle.alive)
    {
        printf("is Null.\n");
        fprintf(log_file, "is Null.\n");
        return;
    }

    printf("Radius = %lf\n", circle.radius);
    printf("Mass = %lf\n", circle.node.mass);
    printf("Position = (%lf, %lf)\n", circle.node.pos.x, circle.node.pos.y);
    printf("Velocity = (%lf, %lf)\n", circle.node.vel.x, circle.node.vel.y);

    fprintf(log_file, "Radius = %lf\n", circle.radius);
    fprintf(log_file, "Mass = %lf\n", circle.node.mass);
    fprintf(log_file, "Position = (%lf, %lf)\n", circle.node.pos.x, circle.node.pos.y);
    fprintf(log_file, "Velocity = (%lf, %lf)\n", circle.node.vel.x, circle.node.vel.y);
}

void createCircle(Uint32 color, double radius, double mass, VECTOR_2D pos, VECTOR_2D vel)
{
    static int id = 1;

    PHYS_OBJ node;
    node.mass = mass;
    node.pos = pos;
    node.vel = vel;

    CIRCLE circle;
    circle.alive = 1;
    circle.id = id++;
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
        if (circles[i].alive)
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
        if (!circles[i].alive)
            continue;
        for (int j = i + 1; j < size; j++)
        {
            if (!circles[j].alive)
                continue;
            double m1 = circles[i].node.mass;
            double m2 = circles[j].node.mass;
            VECTOR_2D u1 = circles[i].node.vel;
            VECTOR_2D u2 = circles[j].node.vel;
            VECTOR_2D pos1 = circles[i].node.pos;
            VECTOR_2D pos2 = circles[j].node.pos;
            double dist = SDL_sqrt(SDL_pow(pos1.x - pos2.x, 2) + SDL_pow(pos1.y - pos2.y, 2));
            if (dist < circles[i].radius + circles[j].radius)
            {
                // Merge circles[j] into circles[i]
                // Conservation of Linear Momentum
                circles[i].node.vel.x = (m1 * u1.x + m2 * u2.x) / (m1 + m2);
                circles[i].node.vel.y = (m1 * u1.y + m2 * u2.y) / (m1 + m2);
                // Conservation of Centre of Mass
                circles[i].node.pos.x = (m1 * pos1.x + m2 * pos2.x) / (m1 + m2);
                circles[i].node.pos.y = (m1 * pos1.y + m2 * pos2.y) / (m1 + m2);
                // mass of new body is the combined mass of both bodies, and radius is recalculated according to new mass
                circles[i].node.mass += circles[j].node.mass;
                circles[i].radius = SDL_sqrt(circles[i].node.mass / (π * DENSITY));
                // destroy circles[j]
                circles[j].alive = 0;
                // stop calculating with circles[i] in current frame in case of merge
                break;
            }
            double force_magnitude = G * m1 * m2 / SDL_pow(dist, 3);
            VECTOR_2D force;
            force.x = force_magnitude * (pos2.x - pos1.x);
            force.y = force_magnitude * (pos2.y - pos1.y);
            circles[i].node.vel.x += force.x / m1 * dt;
            circles[i].node.vel.y += force.y / m1 * dt;
            circles[j].node.vel.x -= force.x / m2 * dt;
            circles[j].node.vel.y -= force.y / m2 * dt;
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
            circles[i].alive = 0;
        else if (circles[i].node.pos.y + circles[i].radius <= 0)
            circles[i].alive = 0;
        else if (circles[i].node.pos.x - circles[i].radius >= WIDTH)
            circles[i].alive = 0;
        else if (circles[i].node.pos.y - circles[i].radius >= HEIGHT)
            circles[i].alive = 0;
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
