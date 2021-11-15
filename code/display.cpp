#include "display.h"
#include <emmintrin.h>
#include <math.h>

static SDL_Window*     window          = NULL;
static SDL_Renderer*   renderer        = NULL;
static SDL_Texture*    texture         = NULL;
static r32*            ZBuffer         = NULL;
static u32*            color_buffer    = NULL;
static u32             window_width    = 320;
static u32             window_height   = 200;


u32 GetWindowWidth(void)
{
    return window_width;
}

u32 GetWindowHeight(void)
{
    return window_height;
}


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
    i32 fullscreen_window_width = 1240;
    i32 fullscreen_window_height = 720;

    window = SDL_CreateWindow(NULL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, fullscreen_window_width, fullscreen_window_height, SDL_WINDOW_SHOWN);//SDL_WINDOW_BORDERLESS);
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

    color_buffer = (u32*)malloc(sizeof(u32)*window_width*window_height);
    ZBuffer      = (r32*)malloc(sizeof(r32)*window_width*window_height);

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, 
                                SDL_TEXTUREACCESS_STREAMING, 
                                window_width, window_height);

    return true;
}

void RenderColorBuffer()
{
    SDL_UpdateTexture(texture, NULL, color_buffer, sizeof(u32)*window_width);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void ClearColorBuffer(u32 color)
{
    for(u32 Y = 0; Y < window_width; ++Y)
    {
        for (u32 X = 0; X < window_width; ++X)
        {
            color_buffer[(window_width*Y) + X] = color;
        }
    }
}

void
PutZBufferPixel(u32 X, u32 Y, r32 V)
{
    if((X > 0) || (Y > 0) || (X < window_width) || (Y < window_height))
    {
        ZBuffer[Y * window_width + X] = V;
    }
}

r32
GetZBufferValue(u32 X, u32 Y)
{
    return ZBuffer[Y * window_width + X];
}

void ClearZBuffer()
{
    for(u32 Y = 0; Y < window_height; ++Y)
    {
        for (u32 X = 0; X < window_width; ++X)
        {
            ZBuffer[(window_width*Y) + X] = 1.0f;
        }
    }
}

void DrawPixel(u32 X, u32 Y, u32 Color)
{
    if(X < window_width || Y < window_height)
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

    for(int PointIdx = 0; PointIdx <= SideLength; ++PointIdx)
    {
        GetBarycentricWeights();
        DrawPixel(roundf(CurrentX), roundf(CurrentY), Color);

        CurrentX += IncX;
        CurrentY += IncY;
    }
}

void DrawPixelFast(__m128 X, __m128 Y, u32 Color)
{
    __m128i Zero = _mm_set1_epi32(0);
    __m128i WindowWidth  = _mm_set1_epi32(window_width);
    __m128i WindowHeight = _mm_set1_epi32(window_height);

    __m128 cmp0 = _mm_and_ps(_mm_cmpge_ps(X, _mm_cvtepi32_ps(Zero)), _mm_cmplt_ps(X, _mm_cvtepi32_ps(WindowWidth)));
    __m128 cmp1 = _mm_and_ps(_mm_cmpge_ps(Y, _mm_cvtepi32_ps(Zero)), _mm_cmplt_ps(Y, _mm_cvtepi32_ps(WindowHeight)));
    if (_mm_movemask_ps(cmp0))
    {
        if (_mm_movemask_ps(cmp1))
        {
            color_buffer[window_width * (u32)((r32*)&Y)[0] + (u32)((r32*)&X)[0]] = Color;
        }
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
    __m128i WindowWidth  = _mm_set1_epi32(window_width);
    __m128i WindowHeight = _mm_set1_epi32(window_height);
    __m128i Zero = _mm_set1_epi32(0.0f);
    __m128i One  = _mm_set1_epi32(1.0f);
    __m128i MinX = _mm_set1_epi32(Min.X);
    __m128i MaxX = _mm_set1_epi32(Max.X);
    __m128i MinY = _mm_set1_epi32(Min.Y);
    __m128i MaxY = _mm_set1_epi32(Max.Y);

    __m128i dX = _mm_sub_epi32(MaxX, MinX);
    __m128i dY = _mm_sub_epi32(MaxY, MinY); 

    __m128i SideLength = _mm_cvtps_epi32(_mm_max_ps(mm_abs_ps(_mm_cvtepi32_ps(dX)), 
                                                    mm_abs_ps(_mm_cvtepi32_ps(dY))));

    __m128 IncX = _mm_setzero_ps();
    __m128 IncY = _mm_setzero_ps();

    __m128 cmpeq0 = _mm_and_ps(_mm_cmpneq_ps(_mm_cvtepi32_ps(dX), _mm_cvtepi32_ps(Zero)), _mm_cmpneq_ps(_mm_cvtepi32_ps(SideLength), _mm_cvtepi32_ps(Zero)));
    __m128 cmpeq1 = _mm_and_ps(_mm_cmpneq_ps(_mm_cvtepi32_ps(dY), _mm_cvtepi32_ps(Zero)), _mm_cmpneq_ps(_mm_cvtepi32_ps(SideLength), _mm_cvtepi32_ps(Zero)));
    if(_mm_movemask_ps(cmpeq0))
    {
        IncX = _mm_div_ps(_mm_cvtepi32_ps(dX), _mm_cvtepi32_ps(SideLength));
    }
    if(_mm_movemask_ps(cmpeq1))
    {
        IncY = _mm_div_ps(_mm_cvtepi32_ps(dY), _mm_cvtepi32_ps(SideLength));
    }

    __m128 CurrentX = _mm_cvtepi32_ps(MinX);
    __m128 CurrentY = _mm_cvtepi32_ps(MinY);

    for(i32 PointIdx = 0; PointIdx <= ((i32*)&SideLength)[0]; PointIdx++)
    {
        __m128i cmp0 = _mm_cvtps_epi32(_mm_and_ps(_mm_cmpge_ps(CurrentX, _mm_cvtepi32_ps(Zero)), _mm_cmplt_ps(CurrentX, _mm_cvtepi32_ps(WindowWidth))));
        __m128i cmp1 = _mm_cvtps_epi32(_mm_and_ps(_mm_cmpge_ps(CurrentY, _mm_cvtepi32_ps(Zero)), _mm_cmplt_ps(CurrentY, _mm_cvtepi32_ps(WindowHeight))));
        if (_mm_movemask_epi8(cmp0))
        {
            if (_mm_movemask_epi8(cmp1))
            {
                color_buffer[window_width * (u32)((r32*)&CurrentY)[0] + (u32)((r32*)&CurrentX)[0]] = Color;
            }
        }

        CurrentX = _mm_add_ps(CurrentX, IncX);
        CurrentY = _mm_add_ps(CurrentY, IncY);
    }
}

void DrawTriangle(triangle_t Triangle, u32 Color)
{
    DrawLine(Triangle.points[0].XY, Triangle.points[1].XY, Color);
    DrawLine(Triangle.points[1].XY, Triangle.points[2].XY, Color);
    DrawLine(Triangle.points[2].XY, Triangle.points[0].XY, Color);
}

void DrawTriangleFast(triangle_t Triangle, u32 Color)
{
    DrawLineFast(Triangle.points[0].XY, Triangle.points[1].XY, Color);
    DrawLineFast(Triangle.points[1].XY, Triangle.points[2].XY, Color);
    DrawLineFast(Triangle.points[2].XY, Triangle.points[0].XY, Color);
}

void DestroyWindow(void)
{
    free(ZBuffer);
    free(color_buffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
