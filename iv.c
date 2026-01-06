#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

typedef struct
{
    int width, height;
} DIMENSIONS;

typedef struct
{
    unsigned char r, g, b;
} RGB_COLOR;

int main(int argc, char **argv)
{
    DIMENSIONS *getImageDimensions(FILE *);
    int getMaxVal(FILE *);
    RGB_COLOR *getRGB(FILE *);

    if (argc != 2)
    {
        printf("Usage: ./iv <PPM file>\n");
        return 0;
    }

    printf("This is an image viewer for raw PPM files\n");
    printf("Viewing File: %s\n", argv[1]);

    FILE *fimg = fopen(argv[1], "rb");
    DIMENSIONS *dims = getImageDimensions(fimg);
    printf("Width: %d, Height: %d\n", dims->width, dims->height);
    int max_val = getMaxVal(fimg);
    printf("Maximum Color Value: %d\n", max_val);

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window *window = SDL_CreateWindow("PPM Image Viewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, dims->width, dims->height, 0);
    SDL_Surface *surface = SDL_GetWindowSurface(window);

    SDL_Rect pixel_rect = {0, 0, 1, 1};
    RGB_COLOR *rgb = NULL;

    for (int y = 0; y < dims->height; y++)
    {
        for (int x = 0; x < dims->width; x++)
        {
            pixel_rect.x = x;
            pixel_rect.y = y;

            rgb = getRGB(fimg);
            int rgb_color_map = SDL_MapRGB(surface->format, rgb->r, rgb->g, rgb->b);
            SDL_FillRect(surface, &pixel_rect, rgb_color_map);
        }
    }

    SDL_UpdateWindowSurface(window);

    int running = 1;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                running = 0;
        }
        SDL_Delay(10);
    }

    fclose(fimg);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

DIMENSIONS *getImageDimensions(FILE *fimg)
{
    DIMENSIONS *dims = (DIMENSIONS *)malloc(sizeof(DIMENSIONS));
    memset(dims, 0, sizeof(dims));

    int buff_byte = 0;

    // Skips Magic Number
    do
    {
        fread(&buff_byte, 1, 1, fimg);
    } while (buff_byte != '\n');

    // Skips Optional Comment Line
    fread(&buff_byte, 1, 1, fimg);
    if (buff_byte == '#')
    {
        do
        {
            fread(&buff_byte, 1, 1, fimg);
        } while (buff_byte != '\n');
    }
    else
    {
        // if there is no comment line, we have read the first character of width
        dims->width = dims->width * 10 + buff_byte - 48;
    }

    while (1)
    {
        // get image width
        fread(&buff_byte, 1, 1, fimg);
        if (buff_byte == ' ')
            break;
        dims->width = dims->width * 10 + buff_byte - 48;
    }

    while (1)
    {
        // get image height
        fread(&buff_byte, 1, 1, fimg);
        if (buff_byte == '\n')
            break;
        dims->height = dims->height * 10 + buff_byte - 48;
    }

    return dims;
}

int getMaxVal(FILE *fimg)
{
    int max_val = 0, buff_byte = 0;
    while (1)
    {
        fread(&buff_byte, 1, 1, fimg);
        if (buff_byte == '\n')
            break;
        max_val = max_val * 10 + buff_byte - 48;
    }
    return max_val;
}

RGB_COLOR *getRGB(FILE *fimg)
{
    RGB_COLOR *rgb = (RGB_COLOR *)malloc(sizeof(RGB_COLOR));
    fread(rgb, 3, 1, fimg);
    return rgb;
}