
static v2
FindTriangleMid(v2 A, v2 B, v2 C)
{
    v2 Result = {};

    Result.Y = B.Y;
    Result.X = A.X + (((B.Y - A.Y)*
                      (C.X - A.X))/
                      (C.Y - A.Y));

    return Result;
}

static void SwapVectors(v2* A, v2* B)
{
    v2 Temp = *A;
    *A = *B;
    *B = Temp;
}

static void FillFlatBottomTriangle(v2 A, v2 B, v2 C, u32 Color) // A(x0, y0), B(x1, y1), C(x2, y2)
{
    r32 Slope1 = (B.X - A.X) / (B.Y - A.Y);
    r32 Slope2 = (C.X - A.X) / (C.Y - A.Y);

    r32 StartX = A.X;
    r32 EndX   = A.X;

    for(r32 Y = A.Y; Y < C.Y; ++Y)
    {
        DrawLine(V2(StartX, Y), V2(EndX, Y), Color);
        StartX += Slope1;
        EndX   += Slope2;
    }
}

static void FillFlatTopTriangle(v2 A, v2 B, v2 C, u32 Color) // A(x0, y0), B(x1, y1), C(x2, y2)
{
    r32 Slope1 = (C.X - A.X) / (C.Y - A.Y);
    r32 Slope2 = (C.X - B.X) / (C.Y - B.Y);

    r32 StartX = C.X;
    r32 EndX   = C.X;

    for(r32 Y = C.Y; Y > A.Y; --Y)
    {
        DrawLine(V2(StartX, Y), V2(EndX, Y), Color);
        StartX -= Slope1;
        EndX   -= Slope2;
    }
}

void DrawFilledTriangle(triangle_t Triangle, u32 Color)
{
    v2 A = Triangle.points[0];
    v2 B = Triangle.points[1];
    v2 C = Triangle.points[2];

    if(A.Y > B.Y) SwapVectors(&A, &B);
    if(B.Y > C.Y) SwapVectors(&B, &C);
    if(A.Y > B.Y) SwapVectors(&A, &B);

    if(B.Y == C.Y) FillFlatBottomTriangle(A, B, C, Color);
    else if(A.Y == B.Y) FillFlatTopTriangle(A, B, C, Color);
    else 
    {
        v2 M = FindTriangleMid(A, B, C);

        // Flat-bottom triangle
        FillFlatBottomTriangle(A, B, M, Color);

        // Flat-top triangle
        FillFlatTopTriangle(B, M, C, Color);
    }
}

