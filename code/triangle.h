#if !defined(TRIANGLE_H)
#define TRIANGLE_H

typedef struct
{
    tex2d A_uv;
    tex2d B_uv;
    tex2d C_uv;
} face_uv_t;

union face_t
{
    struct
    {
        i32 a;
        i32 b;
        i32 c;
        face_uv_t UVs;
        u32 Color;
    };
    i32 E[3];
};

typedef struct
{
    v4 points[3];
    tex2d TextureCoords[3];
    upng_t* Texture;
    u32 Color;
} triangle_t;

inline triangle_t 
Triangle(v3 A, v3 B, v3 C)
{
    triangle_t Result = {};

    Result.points[0] = V4(A, 0.0f);
    Result.points[1] = V4(B, 0.0f);
    Result.points[2] = V4(C, 0.0f);

    return Result;
}

inline triangle_t 
Triangle(v4 A, v4 B, v4 C)
{
    triangle_t Result = {};

    Result.points[0] = A;
    Result.points[1] = B;
    Result.points[2] = C;

    return Result;
}

void DrawFilledTriangle(v2 A, v2 B, v2 C, u32 Color);
void DrawFilledTriangleFast(v2 A, v2 B, v2 C, u32 Color);
void DrawTexturedTriangle(triangle_t Triangle, upng_t* Texture);
v3 GetTriangleNormal(v4 Vertices[3]);

#endif
