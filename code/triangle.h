#if !defined(TRIANGLE_H)
#define TRIANGLE_H

union face_t
{
    struct
    {
        i32 a;
        i32 b;
        i32 c;
        u32 Color;
    };
    i32 E[3];
};

typedef struct
{
    v2 points[3];
    u32 Color;
    r32 AvgDepth;
} triangle_t;

inline 
triangle_t Triangle(v2 A, v2 B, v2 C)
{
    triangle_t Result = {};

    Result.points[0] = A;
    Result.points[1] = B;
    Result.points[2] = C;

    return Result;
}

void DrawFilledTriangle(v2 A, v2 B, v2 C, u32 Color);
void DrawFilledTriangleFast(v2 A, v2 B, v2 C, u32 Color);


#endif
