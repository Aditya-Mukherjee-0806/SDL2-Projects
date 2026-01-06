#include <SDL2/SDL.h>

#define WIDTH 1280
#define HEIGHT 720
#define TILE_SIZE 20
#define ROWS HEIGHT / TILE_SIZE
#define COLS WIDTH / TILE_SIZE

enum TILE_STATE
{
    DEAD,
    LIVE
};

int **tiles_curr;
int **tiles_next;

void InitMatrix(int ***);
void SimulateTiles();
void FillTiles(SDL_Surface *);
int GetLiveNeighbourCount(int **, int, int);

int main()
{
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window *window = SDL_CreateWindow("Conway's Game of Life", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Surface *surface = SDL_GetWindowSurface(window);

    InitMatrix(&tiles_curr);
    InitMatrix(&tiles_next);

    int running = 1, paused = 1;
    while (running)
    {
        SDL_Event event;
        int x = -1, y = -1;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                running = 0;
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_RIGHT)
                    paused = !paused;
                break;
            case SDL_MOUSEMOTION:
                if (event.motion.state & SDL_BUTTON_RMASK)
                    break;

                x = event.motion.x / TILE_SIZE * TILE_SIZE;
                y = event.motion.y / TILE_SIZE * TILE_SIZE;

                if (event.motion.state & SDL_BUTTON_LMASK)
                    tiles_curr[y / TILE_SIZE][x / TILE_SIZE] = LIVE;
                else if (event.motion.state & SDL_BUTTON_MMASK)
                    tiles_curr[y / TILE_SIZE][x / TILE_SIZE] = DEAD;

                break;
            }
        }

        SDL_FillRect(surface, NULL, 0);

        if (!paused)
            SimulateTiles();
        FillTiles(surface);

        SDL_UpdateWindowSurface(window);
        SDL_Delay(100);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

void InitMatrix(int ***matrix)
{
    *matrix = (int **)calloc(ROWS, sizeof(int *));
    for (int i = 0; i < ROWS; i++)
        (*matrix)[i] = (int *)calloc(COLS, sizeof(int));
}

void SimulateTiles()
{
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            int count = GetLiveNeighbourCount(tiles_curr, i, j);
            if (tiles_curr[i][j] == LIVE && (count < 2 || count > 3))
                tiles_next[i][j] = DEAD;
            else if (tiles_curr[i][j] == DEAD && count == 3)
                tiles_next[i][j] = LIVE;
            else
                tiles_next[i][j] = tiles_curr[i][j];
        }
    }

    int **temp = tiles_curr;
    tiles_curr = tiles_next;
    tiles_next = temp;
}

void FillTiles(SDL_Surface *surface)
{
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            SDL_Rect rect = {j * TILE_SIZE, i * TILE_SIZE, TILE_SIZE, TILE_SIZE};
            if (tiles_curr[i][j] == LIVE)
                SDL_FillRect(surface, &rect, 0xffffffff);
            else
                SDL_FillRect(surface, &rect, 0);
        }
    }
}

int GetLiveNeighbourCount(int **tiles, int row, int col)
{
    int count = 0;
    for (int i = row - 1; i <= row + 1; i++)
    {
        if (i < 0 || i >= ROWS)
            continue;
        for (int j = col - 1; j <= col + 1; j++)
        {
            if (i == row && j == col)
                continue;
            if (j < 0 || j >= COLS)
                continue;
            if (tiles[i][j] == LIVE)
                count++;
        }
    }
    return count;
}