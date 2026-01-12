#include <SDL2/SDL.h>

#define WIDTH 1280
#define HEIGHT 720
#define FPS 60
#define COLOR_RECT_SIZE 50
#define BRUSH_SIZE_SMALL 8
#define BRUSH_SIZE_MEDIUM 16
#define BRUSH_SIZE_LARGE 32

#define RGB_BLACK 0, 0, 0
#define RGB_WHITE 255, 255, 255
#define RGB_RED 255, 0, 0
#define RGB_GREEN 0, 255, 0
#define RGB_BLUE 0, 0, 255
#define RGB_YELLOW 255, 255, 0
#define RGB_CYAN 0, 255, 255
#define RGB_MAGENTA 255, 0, 255

typedef struct
{
    Uint8 r, g, b;
} RGB_COLOR;

enum COLORS
{
    BLACK,
    WHITE,
    RED,
    GREEN,
    BLUE,
    YELLOW,
    CYAN,
    MAGENTA,
    PALETTE_SIZE
};

enum BRUSHES
{
    BRUSH_SMALL,
    BRUSH_MEDIUM,
    BRUSH_LARGE,
    BRUSHES_SIZE
};

RGB_COLOR rgb_colors[PALETTE_SIZE];
SDL_Rect color_rects[PALETTE_SIZE];

int brush_sizes[BRUSHES_SIZE];
SDL_Rect brush_rects[BRUSHES_SIZE];

void initPalette();
void initBrushMenu();
void renderPalette(SDL_Renderer *);
void renderBrushMenu(SDL_Renderer *);

int main()
{
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window *window = SDL_CreateWindow("Paint", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    initPalette();
    initBrushMenu();

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    renderPalette(renderer);
    renderBrushMenu(renderer);

    SDL_RenderPresent(renderer);

    RGB_COLOR color_selected = rgb_colors[WHITE];
    SDL_SetRenderDrawColor(renderer, color_selected.r, color_selected.g, color_selected.b, SDL_ALPHA_OPAQUE);
    int brush_size = BRUSH_SIZE_MEDIUM;
    SDL_bool running = SDL_TRUE;
    while (running)
    {
        SDL_Event event;
        SDL_bool draw_occurred = SDL_FALSE;
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
                    SDL_Rect click_pixel = {event.button.x, event.button.y, 1, 1};
                    for (int i = 0; i < PALETTE_SIZE; i++)
                    {
                        if (SDL_HasIntersection(color_rects + i, &click_pixel))
                        {
                            color_selected = rgb_colors[i];
                            printf("Color Selected: %u, %u, %u\n", color_selected.r, color_selected.g, color_selected.b);
                            SDL_SetRenderDrawColor(renderer, color_selected.r, color_selected.g, color_selected.b, SDL_ALPHA_OPAQUE);
                            break;
                        }
                    }
                    for (int i = 0; i < BRUSHES_SIZE; i++)
                    {
                        if (SDL_HasIntersection(brush_rects + i, &click_pixel))
                        {
                            brush_size = brush_sizes[i];
                            printf("Brush Size: %u\n", brush_size);
                        }
                    }
                    break;
                }
                break;
            case SDL_MOUSEMOTION:
                if (event.motion.state & SDL_BUTTON_LMASK)
                {
                    // dont draw over palette or brush menu
                    SDL_Rect draw_rect = {event.motion.x - brush_size / 2, event.motion.y - brush_size / 2, brush_size, brush_size};
                    SDL_Rect palette_rect = {color_rects[0].x, color_rects[0].y, COLOR_RECT_SIZE * PALETTE_SIZE, COLOR_RECT_SIZE};
                    SDL_Rect brush_menu_rect = {brush_rects[0].x, brush_rects[0].y, BRUSH_SIZE_LARGE * BRUSHES_SIZE, BRUSH_SIZE_LARGE};
                    if (!SDL_HasIntersection(&draw_rect, &palette_rect) && !SDL_HasIntersection(&draw_rect, &brush_menu_rect))
                    {
                        SDL_RenderFillRect(renderer, &draw_rect);
                        draw_occurred = SDL_TRUE;
                    }
                }
            }
        }
        if (draw_occurred)
            SDL_RenderPresent(renderer);
        SDL_Delay(1000 / FPS);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

void initPalette()
{
    rgb_colors[BLACK] = (RGB_COLOR){RGB_BLACK};
    rgb_colors[WHITE] = (RGB_COLOR){RGB_WHITE};
    rgb_colors[RED] = (RGB_COLOR){RGB_RED};
    rgb_colors[GREEN] = (RGB_COLOR){RGB_GREEN};
    rgb_colors[BLUE] = (RGB_COLOR){RGB_BLUE};
    rgb_colors[YELLOW] = (RGB_COLOR){RGB_YELLOW};
    rgb_colors[CYAN] = (RGB_COLOR){RGB_CYAN};
    rgb_colors[MAGENTA] = (RGB_COLOR){RGB_MAGENTA};

    for (int i = 0; i < PALETTE_SIZE; i++)
        color_rects[i] = (SDL_Rect){i * COLOR_RECT_SIZE, 0, COLOR_RECT_SIZE, COLOR_RECT_SIZE};
}

void renderPalette(SDL_Renderer *renderer)
{
    for (int i = 0; i < PALETTE_SIZE; i++)
    {
        SDL_SetRenderDrawColor(renderer, rgb_colors[i].r, rgb_colors[i].g, rgb_colors[i].b, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(renderer, color_rects + i);
    }
}

void initBrushMenu()
{
    brush_sizes[BRUSH_SMALL] = BRUSH_SIZE_SMALL;
    brush_sizes[BRUSH_MEDIUM] = BRUSH_SIZE_MEDIUM;
    brush_sizes[BRUSH_LARGE] = BRUSH_SIZE_LARGE;

    int offset_from_right = 0;
    for (int i = BRUSHES_SIZE - 1; i >= 0; i--)
    {
        offset_from_right += brush_sizes[i];
        brush_rects[i] = (SDL_Rect){WIDTH - offset_from_right, 0, brush_sizes[i], brush_sizes[i]};
    }
}

void renderBrushMenu(SDL_Renderer *renderer)
{
    SDL_SetRenderDrawColor(renderer, rgb_colors[WHITE].r, rgb_colors[WHITE].g, rgb_colors[WHITE].b, SDL_ALPHA_OPAQUE);
    for (int i = 0; i < BRUSHES_SIZE; i++)
        SDL_RenderDrawRect(renderer, brush_rects + i);
}