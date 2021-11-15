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
            if((Y % 10 == 0) || (X % 10 == 0)) color_buffer[(window_width*Y) + X] = color;
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
    int dX = Max.X - Min.X;
    int dY = Max.Y - Min.Y; 
    int SideLength = (AbsoluteValue(dX) >= AbsoluteValue(dY)) ? AbsoluteValue(dX) : AbsoluteValue(dY);

    r32 IncX = dX / (r32)SideLength;
    r32 IncY = dY / (r32)SideLength;

    r32 CurrentX = Min.X;
    r32 CurrentY = Min.Y;

    for(int PointIdx = 0; PointIdx <= SideLength; PointIdx++)
    {
        DrawPixel(roundf(CurrentX), roundf(CurrentY), Color);

        CurrentX += IncX;
        CurrentY += IncY;
    }
}

inline __m128
mm_abs_ps(__m128 Val)
{
    __m128 NegMask = _mm_set1_ps(-0.0f);
    __m128 Result = _mm_andnot_ps(NegMask, Val);
    return Result;
}

void DrawLineFast(v2 Min, v2 Max, u32 Color)
{
    __m128 WindowWidth  = _mm_set1_ps(window_width);
    __m128 WindowHeight = _mm_set1_ps(window_height);
    __m128 Zero = _mm_set1_ps(0.0f);
    __m128 One  = _mm_set1_ps(1.0f);
    __m128 MinX = _mm_set1_ps(Min.X);
    __m128 MaxX = _mm_set1_ps(Max.X);
    __m128 MinY = _mm_set1_ps(Min.Y);
    __m128 MaxY = _mm_set1_ps(Max.Y);

    __m128 dX = _mm_sub_ps(MaxX, MinX);
    __m128 dY = _mm_sub_ps(MaxY, MinY); 

    __m128 SideLength = _mm_max_ps(mm_abs_ps(dX), mm_abs_ps(dY));

    __m128 IncX = _mm_div_ps(dX, SideLength);
    __m128 IncY = _mm_div_ps(dY, SideLength);

    __m128 CurrentX = MinX;
    __m128 CurrentY = MinY;

    for(i32 PointIdx = 0; PointIdx <= ((float*)&SideLength)[0]; PointIdx++)
    {
        __m128 cmp0 = _mm_and_ps(_mm_cmpge_ps(CurrentX, Zero), _mm_cmplt_ps(CurrentX, WindowWidth));
        __m128 cmp1 = _mm_and_ps(_mm_cmpge_ps(CurrentY, Zero), _mm_cmplt_ps(CurrentY, WindowHeight));
        cmp0 = _mm_and_ps(cmp0, One);
        cmp1 = _mm_and_ps(cmp1, One);
        if (((float*)&cmp0)[0])
        {
            if (((float*)&cmp1)[0])
            {
                color_buffer[window_width * (uint32_t)((float*)&CurrentY)[0] + (uint32_t)((float*)&CurrentX)[0]] = Color;
            }
        }

        CurrentX = _mm_add_ps(CurrentX, IncX);
        CurrentY = _mm_add_ps(CurrentY, IncY);
    }
}

void DrawTriangle(triangle_t Triangle, u32 Color)
{
    DrawLine(Triangle.points[0], Triangle.points[1], Color);
    DrawLine(Triangle.points[1], Triangle.points[2], Color);
    DrawLine(Triangle.points[2], Triangle.points[0], Color);
}

void DrawTriangleFast(triangle_t Triangle, u32 Color)
{
    DrawLineFast(Triangle.points[0], Triangle.points[1], Color);
    DrawLineFast(Triangle.points[1], Triangle.points[2], Color);
    DrawLineFast(Triangle.points[2], Triangle.points[0], Color);
}

void DestroyWindow(void)
{
    free(color_buffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
