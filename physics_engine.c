#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define WIDTH 1280
#define HEIGHT 720
#define FRAMES_PER_SEC 30
#define DEFAULT_CAPACITY 8
#define PIXELS_PER_METER 1500
#define MIN_RADIUS 15
#define MAX_RADIUS 30
#define DENSITY 500
#define LOG_INTERVAL 1
#define INPUT_BUFFER_SIZE 128

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
    SDL_bool alive;
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
int cap = DEFAULT_CAPACITY, size = 0;
SDL_mutex *circles_mutex;

SDL_bool isPointInsideCircle(VECTOR_2D point, CIRCLE circle);
void displayInfo();
void printCircleInfo(CIRCLE circle);
void createCircle(Uint32 color, double radius, double mass, VECTOR_2D pos, VECTOR_2D vel);
void runSimulation(Uint8 elasticity);
void sanitiseCircles();
void simulateForces(Uint8 elasticity);
void simulateGravitationalForce(Uint8 elasticity);
void handleCollision(CIRCLE *c1, CIRCLE *c2, Uint8 elasticity);
void updatePositions();
void FillCircle(CIRCLE circle);
int waitInput(void *data);
void handleCreateInput(char *input);
void handleClearInput(char *input);
void handleSetInput(char *input);
CIRCLE *findCircleById(int id);

int main()
{
    srand(time(NULL));
    log_file = fopen("phys_sim_log.txt", "w");
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window *window = SDL_CreateWindow("Physics Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    surface = SDL_GetWindowSurface(window);

    circles = (CIRCLE *)calloc(cap, sizeof(CIRCLE));

    const Uint32 colors[] = {
        SDL_MapRGB(surface->format, RGB_RED),
        SDL_MapRGB(surface->format, RGB_GREEN),
        SDL_MapRGB(surface->format, RGB_BLUE),
        SDL_MapRGB(surface->format, RGB_YELLOW),
        SDL_MapRGB(surface->format, RGB_CYAN),
        SDL_MapRGB(surface->format, RGB_MAGENTA),
    };
    const int num_colors = sizeof(colors) / sizeof(colors[0]);

    circles_mutex = SDL_CreateMutex();
    SDL_Thread *input_thread = SDL_CreateThread(waitInput, "input thread", (void *)colors);

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

    Uint8 elasticity = 1;
    int frames = 0;
    double frame_time_sum = 0, max_frame_time = 0, min_frame_time = dt;
    SDL_bool running = SDL_TRUE;
    while (running)
    {
        Uint64 start = SDL_GetPerformanceCounter();
        frames++;
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                running = SDL_FALSE;
                break;
            case SDL_MOUSEBUTTONDOWN:
                switch (event.button.button)
                {
                case SDL_BUTTON_LEFT:
                    VECTOR_2D point = {event.button.x, event.button.y};
                    for (int i = 0; i < size; i++)
                    {
                        if (isPointInsideCircle(point, circles[i]))
                        {
                            printf("ID: %d\n", circles[i].id);
                            fflush(stdout);
                            break;
                        }
                    }
                    break;
                }
                break;
            }
        }
        SDL_FillRect(surface, NULL, 0);
        runSimulation(elasticity);
        SDL_UpdateWindowSurface(window);
        if (frames % (LOG_INTERVAL * FRAMES_PER_SEC) == 0)
            displayInfo();
        Uint64 end = SDL_GetPerformanceCounter();
        double frame_time = (double)(end - start) / SDL_GetPerformanceFrequency();
        frame_time_sum += frame_time;
        if (frame_time < min_frame_time)
            min_frame_time = frame_time;
        if (frame_time > max_frame_time)
            max_frame_time = frame_time;
        if (frame_time < dt)
            SDL_Delay((dt - frame_time) * 1000);
    }

    printf("Number of frames: %d\n", frames);
    printf("Time passed: %lf\n", (double)frames / FRAMES_PER_SEC);
    printf("Avg. Frame Time: %lf\n", frame_time_sum / frames);
    printf("Min. Frame Time: %lf\n", min_frame_time);
    printf("Max. Frame Time: %lf\n", max_frame_time);

    SDL_FreeSurface(surface);
    SDL_DestroyMutex(circles_mutex);
    SDL_DestroyWindow(window);
    SDL_Quit();
    fclose(log_file);
    free(circles);
    return 0;
}

SDL_bool isPointInsideCircle(VECTOR_2D point, CIRCLE circle)
{
    double dist_sq = SDL_pow(point.x - circle.node.pos.x, 2) + SDL_pow(point.y - circle.node.pos.y, 2);
    return dist_sq <= circle.radius * circle.radius;
}

void displayInfo()
{
    static int log_count = 1;
    // printf("Log Entry: #%d\n", log_count);
    fprintf(log_file, "ENTRY: #%d\n", log_count);
    for (int i = 0; i < size; i++)
    {
        // printf("Circle %d:\n", circles[i].id);
        fprintf(log_file, "Circle %d:\n", circles[i].id);
        printCircleInfo(circles[i]);
    }
    log_count++;
}

void printCircleInfo(CIRCLE circle)
{
    if (!circle.alive)
    {
        // printf("is Null.\n");
        fprintf(log_file, "is Null.\n");
        return;
    }

    // printf("Radius = %lf\n", circle.radius);
    // printf("Mass = %lf\n", circle.node.mass);
    // printf("Position = (%lf, %lf)\n", circle.node.pos.x, circle.node.pos.y);
    // printf("Velocity = (%lf, %lf)\n", circle.node.vel.x, circle.node.vel.y);

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

    SDL_LockMutex(circles_mutex);

    if (size >= cap)
    {
        CIRCLE *temp = (CIRCLE *)realloc(circles, cap * 2 * sizeof(CIRCLE));
        if (temp)
        {
            cap *= 2;
            circles = temp;
            // memset(circles + size, 0, (cap - size) * sizeof(CIRCLE));
        }
        else
            fprintf(stderr, "REALLOCATION FAILED in %s\n", __func__);
    }
    circles[size++] = circle;

    SDL_UnlockMutex(circles_mutex);
}

void runSimulation(Uint8 elasticity)
{
    SDL_LockMutex(circles_mutex);

    sanitiseCircles();
    simulateForces(elasticity);
    updatePositions();
    for (int i = 0; i < size; i++)
        FillCircle(circles[i]);

    SDL_UnlockMutex(circles_mutex);
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

    if (size < cap / 4)
    {
        CIRCLE *temp = (CIRCLE *)realloc(circles, cap / 2 * sizeof(CIRCLE));
        if (temp)
        {
            cap /= 2;
            circles = temp;
            // memset(circles + size, 0, (cap - size) * sizeof(CIRCLE));
        }
        else
            fprintf(stderr, "REALLOCATION FAILED in %s\n", __func__);
    }
}

void simulateForces(Uint8 elasticity)
{
    simulateGravitationalForce(elasticity);
}

void simulateGravitationalForce(Uint8 elasticity)
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
                handleCollision(circles + i, circles + j, elasticity);

                if (elasticity == 0)
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

void handleCollision(CIRCLE *c1, CIRCLE *c2, Uint8 elasticity)
{
    double m1 = c1->node.mass;
    double m2 = c2->node.mass;
    VECTOR_2D u1 = c1->node.vel;
    VECTOR_2D u2 = c2->node.vel;
    VECTOR_2D pos1 = c1->node.pos;
    VECTOR_2D pos2 = c2->node.pos;
    if (elasticity == 1)
    {
        // bounce c1 and c2 off each other
        c1->node.vel.x = ((m1 - m2) * u1.x + 2 * m2 * u2.x) / (m1 + m2);
        c1->node.vel.y = ((m1 - m2) * u1.y + 2 * m2 * u2.y) / (m1 + m2);

        c2->node.vel.x = ((m2 - m1) * u2.x + 2 * m1 * u1.x) / (m1 + m2);
        c2->node.vel.y = ((m2 - m1) * u2.y + 2 * m1 * u1.y) / (m1 + m2);
    }
    else
    {
        // Merge c2 into c1
        // Conservation of Linear Momentum
        c1->node.vel.x = (m1 * u1.x + m2 * u2.x) / (m1 + m2);
        c1->node.vel.y = (m1 * u1.y + m2 * u2.y) / (m1 + m2);
        // Conservation of Centre of Mass
        c1->node.pos.x = (m1 * pos1.x + m2 * pos2.x) / (m1 + m2);
        c1->node.pos.y = (m1 * pos1.y + m2 * pos2.y) / (m1 + m2);
        // mass of new body is the combined mass of both bodies, and radius is recalculated according to new mass
        c1->node.mass += c2->node.mass;
        c1->radius = SDL_sqrt(c1->node.mass / (π * DENSITY));
        // destroy c2
        c2->alive = 0;
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

int SDLCALL waitInput(void *data)
{
    const Uint32 *colors = (Uint32 *)(data);
    printf("Supported Commands: create, clear, set\n");
    while (1)
    {
        char input[INPUT_BUFFER_SIZE];
        char command[10];
        printf("Reading input...\n");
        fgets(input, INPUT_BUFFER_SIZE, stdin);
        sscanf(input, "%s", command);
        if (strncasecmp(command, "create", sizeof command) == 0)
            handleCreateInput(input);
        else if (strncasecmp(command, "clear", sizeof command) == 0)
            handleClearInput(input);
        else if (strncasecmp(command, "set", sizeof command) == 0)
            handleSetInput(input);
        else
            printf("%s is not a supported command\n", command);
        // char *command = strtok(input, " ");
        // if (strcasecmp(command, "create") != 0)
        // {
        //     printf("Usage: create <color> <radius> <mass> <posx> <posy> <velx> <vely>\n");
        //     continue;
        // }

        // char *color_tok = strtok(NULL, " ");
        // int color_index = -1;
        // switch (*color_tok)
        // {
        // case 'r':
        //     color_index = 0;
        //     break;
        // case 'g':
        //     color_index = 1;
        //     break;
        // case 'b':
        //     color_index = 2;
        //     break;
        // case 'y':
        //     color_index = 3;
        //     break;
        // case 'c':
        //     color_index = 4;
        //     break;
        // case 'm':
        //     color_index = 5;
        //     break;
        // }

        // double radius = atof(strtok(NULL, " "));
        // double mass = atof(strtok(NULL, " "));
        // VECTOR_2D pos = {atof(strtok(NULL, " ")), atof(strtok(NULL, " "))};
        // VECTOR_2D vel = {atof(strtok(NULL, " ")), atof(strtok(NULL, " "))};

        // createCircle(colors[color_index], radius, mass, pos, vel);
    }
}

void handleCreateInput(char *input)
{
}

void handleClearInput(char *input)
{
    SDL_LockMutex(circles_mutex);
    char *delims = " \t\r\n";
    strtok(input, delims); // discard the command
    char *flag = strtok(NULL, delims);
    if (flag == NULL || strcasecmp(flag, "--all") == 0)
    {
        // clear the object array before the next frame
        size = 0;
        cap = DEFAULT_CAPACITY;
    }
    else if (strcasecmp(flag, "--id") == 0)
    {
        char *id_str = strtok(NULL, delims);
        if (id_str == NULL)
            printf("ID not provided\n");
        else
        {
            int id;
            if (sscanf(id_str, "%d", &id) != 1)
                printf("ID field is invalid\n");
            else
            {
                CIRCLE *circleFound = findCircleById(id);
                if (circleFound != NULL)
                    circleFound->alive = SDL_FALSE;
                else
                    printf("Circle with ID: %d does not exist\n", id);
            }
        }
    }
    else
        printf("Invalid Flag: %s\n", flag);

    SDL_UnlockMutex(circles_mutex);
}

void handleSetInput(char *input)
{
}

CIRCLE *findCircleById(int id)
{
    for (int i = 0; i < size; i++)
    {
        if (circles[i].id == id)
            return circles + i;
    }
    return NULL;
}