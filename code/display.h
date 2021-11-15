#if !defined(DISPLAY_H_)
#define _USE_MATH_DEFINES

#include <SDL2\SDL.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <intrin.h>

typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

typedef int8_t      i8;
typedef int16_t     i16;
typedef int32_t     i32;
typedef uint64_t    i64;

typedef float       r32;
typedef double      r64;

#define FPS 30
#define FRAME_TARGET_TIME (1000/FPS)

#include "upng.h"
#include "hmath.h"
#include "matrix.h"
#include "texture.h"
#include "triangle.h"
#include "mesh.h"
#include "array.h"
#include "Camera.h"
#include "clipping.h"

enum display_type
{
    DisplayType_Dots            = (1<<0),
    DisplayType_Lines           = (1<<1),
    DisplayType_FilledTriangles = (1<<2),
    DisplayType_Culling         = (1<<3),
    DisplayType_Textured        = (1<<4),
};

bool InitWindow();
void RenderColorBuffer();
void ClearColorBuffer(u32);
void ClearZBuffer();
void DrawPixel(u32, u32, u32);
void DrawGrid(u32);
void DrawRect(v2, u32, u32, u32);
void DrawLine(v2, v2, u32);
void DrawPixelFast(__m128 X, __m128 Y, u32 Color);
void DrawLineFast(v2, v2, u32);
void DrawTriangle(triangle_t, u32);
void DrawTriangleFast(triangle_t, u32);
void DestroyWindow();


u32 GetWindowWidth(void);
u32 GetWindowHeight(void);
void PutZBufferPixel(u32 X, u32 Y, r32 V);
r32 GetZBufferValue(u32 X, u32 Y);

#define DISPLAY_H_
#endif
