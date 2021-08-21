#include "display.h"
#include <math.h>

SDL_Window*     window          = NULL;
SDL_Renderer*   renderer        = NULL;
SDL_Texture*    texture         = NULL;
u32*            color_buffer    = NULL;
u32             window_width    = 1240;
u32             window_height   = 720;


bool InitWindow(void)
{
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        fprintf(stderr, "Error: initializing SDL\n");
        return false;
    }

    SDL_DisplayMode display_mode;
    SDL_GetCurrentDisplayMode(0, &display_mode);

    //window_width  = display_mode.w;
    //window_height = display_mode.h;
    window_width = 1240;
    window_height = 720;

    window = SDL_CreateWindow(NULL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, SDL_WINDOW_SHOWN);//SDL_WINDOW_BORDERLESS);
    if(!window)
    {
        fprintf(stderr, "Error: creating SDL window");
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, NULL);
    if(!renderer)
    {
        fprintf(stderr, "Error: creating SDL renderer");
        return false;
    }

    //SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

    return true;
}

void RenderColorBuffer()
{
    SDL_UpdateTexture(texture, NULL, color_buffer, sizeof(u32)*window_width);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
}

void ClearColorBuffer(u32 color)
{
    for(u32 Y = 0; Y < window_height; ++Y)
    {
        for (u32 X = 0; X < window_width; ++X)
        {
            color_buffer[(window_width*Y) + X] = color;
        }
    }
}

void DrawPixel(u32 X, u32 Y, u32 Color)
{
    if(X < window_width && Y < window_height)
    {
        color_buffer[window_width*Y + X] = Color;
    }
}

void DrawGrid(u32 color)
{
    for(u32 Y = 0; (Y < window_height); ++Y)
    {
        for(u32 X = 0; (X < window_width); ++X)
        {
            if((Y % 20 == 0) || (X % 20 == 0)) color_buffer[(window_width*Y) + X] = color;
        }
    }
}

void DrawRect(v2 Min, u32 width, u32 height, u32 color)
{
    u32 MinX = Min.X;
    u32 MinY = Min.Y;
    u32 MaxX = MinX+width;
    u32 MaxY = MinY+height;

    if(MinX < 0) return;  //MinX = 0;
    if(MinY < 0) return;  //MinY = 0;
    if(MaxX > window_width) return;   //MaxX = window_width;
    if(MaxY > window_height) return;  //MaxY = window_height;

    u32 BytesPerPixel = sizeof(u32);
    u32 Pitch = BytesPerPixel * window_width;

    u8* Row = ((u8*)color_buffer + 
                    MinX*BytesPerPixel + 
                    MinY*Pitch);

    for(u32 Y = MinY; Y < MaxY; ++Y)
    {
        u32* Pixel = (u32*)Row;
        for(u32 X = MinX; X < MaxX; ++X)
        {
            *Pixel++ = color;
        }
        Row += Pitch;
    }
}

void DrawLine(v2 Min, v2 Max, u32 Color)
{
    r32 dX = Max.X - Min.X;
    r32 dY = Max.Y - Min.Y; 
    u32 SideLength = (AbsoluteValue(dX) > AbsoluteValue(dY)) ? AbsoluteValue(dX) : AbsoluteValue(dY);

    r32 IncX = dX / (r32)SideLength;
    r32 IncY = dY / (r32)SideLength;

    r32 CurrentX = Min.X;
    r32 CurrentY = Min.Y;

    for(u32 PointIdx = 0; PointIdx < SideLength; ++PointIdx)
    {
        DrawPixel(roundf(CurrentX), roundf(CurrentY), Color);

        CurrentX += IncX;
        CurrentY += IncY;
    }
}

void DrawTriangle(triangle_t Triangle, u32 Color)
{
    DrawLine(Triangle.points[0], Triangle.points[1], Color);
    DrawLine(Triangle.points[1], Triangle.points[2], Color);
    DrawLine(Triangle.points[2], Triangle.points[0], Color);
}

void DestroyWindow(void)
{
    free(color_buffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
