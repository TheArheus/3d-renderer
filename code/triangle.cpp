#include <emmintrin.h>

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
    r32 Slope1 = (r32)((int)B.X - (int)A.X) / ((int)B.Y - (int)A.Y);
    r32 Slope2 = (r32)((int)C.X - (int)A.X) / ((int)C.Y - (int)A.Y);

    r32 StartX = (int)A.X;
    r32 EndX   = (int)A.X;

    for(int Y = A.Y; Y <= (int)C.Y; Y++)
    {
        DrawLine(V2(StartX, Y), V2(EndX, Y), Color);
        StartX += Slope1;
        EndX   += Slope2;
    }
}

static void FillFlatTopTriangle(v2 A, v2 B, v2 C, u32 Color) // A(x0, y0), B(x1, y1), C(x2, y2)
{
    r32 Slope1 = (r32)((int)C.X - (int)A.X) / ((int)C.Y - (int)A.Y);
    r32 Slope2 = (r32)((int)C.X - (int)B.X) / ((int)C.Y - (int)B.Y);

    r32 StartX = (int)C.X;
    r32 EndX   = (int)C.X;

    for(int Y = C.Y; Y >= (int)A.Y; Y--)
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

inline __m128
mm_abs_ps(__m128 Val)
{
    __m128 NegMask = _mm_set1_ps(-0.0f);
    __m128 Result = _mm_andnot_ps(NegMask, Val);
    return Result;
}

// TODO: Correct not filled lines inside
void DrawFilledTriangleFast(triangle_t Triangle, uint32_t Color)
{
    __m128 WindowWidth  = _mm_set1_ps(window_width);
    __m128 WindowHeight = _mm_set1_ps(window_height);
    __m128 Zero = _mm_set1_ps(0.0f);
    __m128 One  = _mm_set1_ps(1.0f);

    __m128 Ax  = _mm_set1_ps(Triangle.points[0].X);
    __m128 Ay  = _mm_set1_ps(Triangle.points[0].Y);
    __m128 Bx  = _mm_set1_ps(Triangle.points[1].X);
    __m128 By  = _mm_set1_ps(Triangle.points[1].Y);
    __m128 Cx  = _mm_set1_ps(Triangle.points[2].X);
    __m128 Cy  = _mm_set1_ps(Triangle.points[2].Y);

    __m128 cmp0 = _mm_cmpgt_ps(Ay, By);
    if(((float*)&_mm_and_ps(cmp0, One))[0])
    {
        Ax = _mm_xor_ps(Ax, Bx);
        Bx = _mm_xor_ps(Bx, Ax);
        Ax = _mm_xor_ps(Ax, Bx);

        Ay = _mm_xor_ps(Ay, By);
        By = _mm_xor_ps(By, Ay);
        Ay = _mm_xor_ps(Ay, By);
    } // SwapVectors(&A, &B);
    __m128 cmp1 = _mm_cmpgt_ps(By, Cy);
    if(((float*)&_mm_and_ps(cmp1, One))[0])
    {
        Bx = _mm_xor_ps(Bx, Cx);
        Cx = _mm_xor_ps(Cx, Bx);
        Bx = _mm_xor_ps(Bx, Cx);
                          
        By = _mm_xor_ps(By, Cy);
        Cy = _mm_xor_ps(Cy, By);
        By = _mm_xor_ps(By, Cy);
    } // SwapVectors(&B, &C);
    cmp0 = _mm_cmpgt_ps(Ay, By);
    if(((float*)&_mm_and_ps(cmp0, One))[0])
    {
        Ax = _mm_xor_ps(Ax, Bx);
        Bx = _mm_xor_ps(Bx, Ax);
        Ax = _mm_xor_ps(Ax, Bx);
                          
        Ay = _mm_xor_ps(Ay, By);
        By = _mm_xor_ps(By, Ay);
        Ay = _mm_xor_ps(Ay, By);
    } // SwapVectors(&A, &B);

    cmp0 = _mm_and_ps(_mm_cmpeq_ps(Ay, By), One);
    cmp1 = _mm_and_ps(_mm_cmpeq_ps(By, Cy), One);
    if(((float*)&cmp1)[0]) 
    {
        __m128 Slope1 = _mm_div_ps(_mm_sub_ps(Bx, Ax), _mm_sub_ps(By, Ay));
        __m128 Slope2 = _mm_div_ps(_mm_sub_ps(Cx, Ax), _mm_sub_ps(Cy, Ay));

        __m128 StartX = Ax;
        __m128 EndX   = Ax;

        for(float Y = ((float*)&Ay)[0]; Y < ((float*)&Cy)[0]; ++Y)
        {
            __m128 MinX = StartX;
            __m128 MinY = _mm_set1_ps((float)Y);
            __m128 MaxX = EndX;
            __m128 MaxY = _mm_set1_ps((float)Y);

            __m128 dX = _mm_sub_ps(MaxX, MinX);
            __m128 dY = _mm_sub_ps(MaxY, MinY); 

            __m128 SideLength = _mm_max_ps(mm_abs_ps(dX), mm_abs_ps(dY));

            __m128 IncX = _mm_div_ps(dX, SideLength);
            __m128 IncY = _mm_div_ps(dY, SideLength);

            __m128 CurrentX = MinX;
            __m128 CurrentY = MinY;

            for(uint32_t PointIdx = 0; PointIdx < ((float*)&SideLength)[0]; ++PointIdx)
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

            StartX = _mm_add_ps(StartX, Slope1);
            EndX   = _mm_add_ps(EndX,   Slope2);
        }
    } // FillFlatBottomTriangle(A, B, C, Color);
    else if(((float*)&cmp0)[0])
    {
        __m128 Slope1 = _mm_div_ps(_mm_sub_ps(Cx, Ax), _mm_sub_ps(Cy, Ay));
        __m128 Slope2 = _mm_div_ps(_mm_sub_ps(Cx, Bx), _mm_sub_ps(Cy, By));

        __m128 StartX = Cx;
        __m128 EndX   = Cx;

        for(float Y = ((float*)&Cy)[0]; Y > ((float*)&Ay)[0]; --Y)
        {
            __m128 MinX = StartX;
            __m128 MinY = _mm_set1_ps((float)Y);
            __m128 MaxX = EndX;
            __m128 MaxY = _mm_set1_ps((float)Y);

            __m128 dX = _mm_sub_ps(MaxX, MinX);
            __m128 dY = _mm_sub_ps(MaxY, MinY); 

            __m128 SideLength = _mm_max_ps(mm_abs_ps(dX), mm_abs_ps(dY));

            __m128 IncX = _mm_div_ps(dX, SideLength);
            __m128 IncY = _mm_div_ps(dY, SideLength);

            __m128 CurrentX = MinX;
            __m128 CurrentY = MinY;

            for(uint32_t PointIdx = 0; PointIdx < ((float*)&SideLength)[0]; ++PointIdx)
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

            StartX = _mm_sub_ps(StartX, Slope1);
            EndX   = _mm_sub_ps(EndX, Slope2);
        }
    } // FillFlatTopTriangle(A, B, C, Color);
    else 
    {
        // Find Triangle Mid Point
        //v2 M = FindTriangleMid(A, B, C);
        __m128 My = By;
        __m128 Mx = _mm_add_ps(Ax, _mm_div_ps(_mm_mul_ps(_mm_sub_ps(By, Ay), _mm_sub_ps(Cx, Ax)), _mm_sub_ps(Cy, Ay)));

#if 1
        // Flat-bottom triangle
        // A = A, B = M, C = B
        __m128 Slope1 = _mm_div_ps(_mm_sub_ps(Bx, Ax), _mm_sub_ps(By, Ay));
        __m128 Slope2 = _mm_div_ps(_mm_sub_ps(Mx, Ax), _mm_sub_ps(My, Ay));

        __m128 StartX = Ax;
        __m128 EndX   = Ax;

        for(float Y = ((float*)&Ay)[0]; Y < ((float*)&My)[0]; ++Y)
        {
            __m128 MinX = StartX;
            __m128 MinY = _mm_set1_ps((float)Y);
            __m128 MaxX = EndX;
            __m128 MaxY = _mm_set1_ps((float)Y);

            __m128 dX = _mm_sub_ps(MaxX, MinX);
            __m128 dY = _mm_sub_ps(MaxY, MinY); 

            __m128 SideLength = _mm_max_ps(mm_abs_ps(dX), mm_abs_ps(dY));

            __m128 IncX = _mm_div_ps(dX, SideLength);
            __m128 IncY = _mm_div_ps(dY, SideLength);

            __m128 CurrentX = MinX;
            __m128 CurrentY = MinY;

            for(uint32_t PointIdx = 0; PointIdx < ((float*)&SideLength)[0]; ++PointIdx)
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

            StartX = _mm_add_ps(StartX, Slope1);
            EndX   = _mm_add_ps(EndX,   Slope2);
        }

        // Flat-top triangle
        // A = B, B = M, C = C
        Slope1 = _mm_div_ps(_mm_sub_ps(Cx, Bx), _mm_sub_ps(Cy, By));
        Slope2 = _mm_div_ps(_mm_sub_ps(Cx, Mx), _mm_sub_ps(Cy, My));

        StartX = Cx;
        EndX   = Cx;

        for(float Y = ((float*)&Cy)[0]; Y > ((float*)&By)[0]; --Y)
        {
            __m128 MinX = StartX;
            __m128 MinY = _mm_set1_ps((float)Y);
            __m128 MaxX = EndX;
            __m128 MaxY = _mm_set1_ps((float)Y);

            __m128 dX = _mm_sub_ps(MaxX, MinX);
            __m128 dY = _mm_sub_ps(MaxY, MinY); 

            __m128 SideLength = _mm_max_ps(mm_abs_ps(dX), mm_abs_ps(dY));

            __m128 IncX = _mm_div_ps(dX, SideLength);
            __m128 IncY = _mm_div_ps(dY, SideLength);

            __m128 CurrentX = MinX;
            __m128 CurrentY = MinY;

            for(uint32_t PointIdx = 0; PointIdx < ((float*)&SideLength)[0]; ++PointIdx)
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

            StartX = _mm_sub_ps(StartX, Slope1);
            EndX   = _mm_sub_ps(EndX,   Slope2);
        }
#endif
    }
}

