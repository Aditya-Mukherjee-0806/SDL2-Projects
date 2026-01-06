#include <SDL2/SDL.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH 800
#define HEIGHT 600
#define TILE_SIZE 25
#define ROWS HEIGHT / TILE_SIZE
#define COLS WIDTH / TILE_SIZE
#define FRAME_RATE 4
#define RGB_SNAKE 0, 255, 0
#define RGB_FOOD 255, 0, 0
#define RGB_BG 0, 0, 0
#define START_POS 200, 200

typedef struct NODE
{
    int x, y;
    struct NODE *next, *prev;
} NODE;

typedef enum
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
} DIR;

typedef enum
{
    EMPTY,
    SNAKE,
    FOOD,
} TILE_STATE;

NODE *head = NULL, *tail = NULL;
int *tiles;

void InsertFirstAndFill(SDL_Surface *, int, int);
void DeleteLastAndClear(SDL_Surface *);
void GenerateAndFillFoodCoords(SDL_Surface *, int *, int *, int *);
int GetIndexFromCoords(int, int);

int main()
{
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window *window = SDL_CreateWindow("Snake", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Surface *surface = SDL_GetWindowSurface(window);

    srand(time(NULL));
    tiles = (int *)calloc(ROWS * COLS, sizeof(int));

    InsertFirstAndFill(surface, START_POS);
    DIR dir = RIGHT;
    int food_x, food_y;
    GenerateAndFillFoodCoords(surface, tiles, &food_x, &food_y);

    int running = 1, score = 0;
    while (running)
    {
        SDL_Event event;
        int next_x = head->x, next_y = head->y;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                running = 0;
            else if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_w:
                case SDLK_UP:
                    dir = UP;
                    break;
                case SDLK_a:
                case SDLK_LEFT:
                    dir = LEFT;
                    break;
                case SDLK_s:
                case SDLK_DOWN:
                    dir = DOWN;
                    break;
                case SDLK_d:
                case SDLK_RIGHT:
                    dir = RIGHT;
                    break;
                }
            }
        }

        switch (dir)
        {
        case UP:
            next_y -= TILE_SIZE;
            break;
        case DOWN:
            next_y += TILE_SIZE;
            break;
        case LEFT:
            next_x -= TILE_SIZE;
            break;
        case RIGHT:
            next_x += TILE_SIZE;
            break;
        }

        if (next_x >= WIDTH || next_x < 0 || next_y >= HEIGHT || next_y < 0 || tiles[GetIndexFromCoords(next_x, next_y)] == SNAKE)
            break;

        InsertFirstAndFill(surface, next_x, next_y);

        if (head->x == food_x && head->y == food_y)
        {
            GenerateAndFillFoodCoords(surface, tiles, &food_x, &food_y);
            score++;
        }
        else
            DeleteLastAndClear(surface);

        SDL_UpdateWindowSurface(window);
        SDL_Delay(1000 / FRAME_RATE);
    }

    printf("GAME OVER!\nScore = %d\n", score);
    SDL_Delay(1000);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

void InsertFirstAndFill(SDL_Surface *surface, int x, int y)
{
    NODE *p = (NODE *)malloc(sizeof(NODE));
    p->x = x;
    p->y = y;
    p->next = p->prev = NULL;

    if (!head && !tail)
    {
        head = p;
        tail = p;
    }
    else
    {
        p->next = head;
        head->prev = p;
        head = p;
    }

    tiles[GetIndexFromCoords(head->x, head->y)] = SNAKE;

    SDL_Rect rect = {head->x, head->y, TILE_SIZE, TILE_SIZE};
    SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, RGB_SNAKE));
}

void DeleteLastAndClear(SDL_Surface *surface)
{
    if (!tail)
        return;

    SDL_Rect rect = {tail->x, tail->y, TILE_SIZE, TILE_SIZE};
    SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, RGB_BG));

    tiles[GetIndexFromCoords(tail->x, tail->y)] = EMPTY;

    if (head == tail)
    {
        free(tail);
        head = tail = NULL;
        return;
    }

    NODE *p = tail;
    tail = tail->prev;
    tail->next = NULL;
    p->prev = NULL;
    free(p);
}

void GenerateAndFillFoodCoords(SDL_Surface *surface, int *tiles, int *food_x, int *food_y)
{
    int x, y;
    do
    {
        x = rand() % COLS * TILE_SIZE;
        y = rand() % ROWS * TILE_SIZE;
    } while (tiles[GetIndexFromCoords(x, y)] != EMPTY);

    SDL_FillRect(surface, &(SDL_Rect){x, y, TILE_SIZE, TILE_SIZE}, SDL_MapRGB(surface->format, RGB_FOOD));
    *food_x = x;
    *food_y = y;
    tiles[GetIndexFromCoords(x, y)] = FOOD;
}

int GetIndexFromCoords(int x, int y)
{
    return y / TILE_SIZE * COLS + x / TILE_SIZE;
}