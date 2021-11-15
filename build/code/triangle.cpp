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


#define SwapTwoObjects_(type) void SwapTwo_##type(type* A, type* B) \
{                                                                   \
    type Temp = *A;                                                 \
    *A = *B;                                                        \
    *B = Temp;                                                      \
}

SwapTwoObjects_(v2);
SwapTwoObjects_(v4);
SwapTwoObjects_(tex2d);

v3 GetTriangleNormal(v4 Vertices[3])
{
    v3 A = Vertices[0].XYZ;
    v3 B = Vertices[1].XYZ;
    v3 C = Vertices[2].XYZ;

    v3 AB = B - A; // B - A
    v3 AC = C - A; // C - A
    AB *= (1.0f / Length(AB));
    AC *= (1.0f / Length(AC));
    v3 Normal = Cross(AB, AC);
    Normal *= (1.0f / Length(Normal));

    return Normal;
}

static void
DrawTrianglePixel(i32 X, i32 Y, 
                  v4 A, v4 B, v4 C, 
                  u32 Color)
{
    v2 P = V2(X, Y);

    v3 Bary = GetBarycentricWeights(A.XY, B.XY, C.XY, P);

    r32 ReciprocalW = (1.0f / A.W) * Bary.X + (1.0f / B.W) * Bary.Y + (1.0f / C.W) * Bary.Z;
    ReciprocalW = 1.0f - ReciprocalW;

    if (ReciprocalW < GetZBufferValue(X, Y))
    {
        DrawPixel(X, Y, Color);
        PutZBufferPixel(X, Y, ReciprocalW);
    }
}

void DrawFilledTriangle(triangle_t Triangle, u32 Color)
{
    v4 A = Triangle.points[0];
    v4 B = Triangle.points[1];
    v4 C = Triangle.points[2];

    if(A.Y > B.Y) SwapTwo_v4(&A, &B);
    if(B.Y > C.Y) SwapTwo_v4(&B, &C);
    if(A.Y > B.Y) SwapTwo_v4(&A, &B);

    int Ax = A.X;
    int Ay = A.Y;
    int Bx = B.X;
    int By = B.Y;
    int Cx = C.X;
    int Cy = C.Y;

    r32 InvSlope1 = 0;
    r32 InvSlope2 = 0;
    
    if((By - Ay) != 0) InvSlope1 = (r32)(Bx - Ax) / AbsoluteValue(By - Ay);
    if((Cy - Ay) != 0) InvSlope2 = (r32)(Cx - Ax) / AbsoluteValue(Cy - Ay);

    if((By - Ay) != 0)
    {
        for(i32 Y = Ay; Y <= By; ++Y)
        {
            i32 StartX = Bx + InvSlope1*(Y - By);
            i32 EndX   = Ax + InvSlope2*(Y - Ay);

            if(EndX < StartX)
            {
                i32 Temp = StartX;
                StartX = EndX;
                EndX = Temp;
            }

            for(i32 X = StartX; X <= EndX; ++X)
            {
                DrawTrianglePixel(X, Y, A, B, C, Color);
            }
        }
    }

    InvSlope1 = 0;
    InvSlope2 = 0;

    if((Cy - By) != 0) InvSlope1 = (r32)(Cx - Bx) / AbsoluteValue(Cy - By);
    if((Cy - Ay) != 0) InvSlope2 = (r32)(Cx - Ax) / AbsoluteValue(Cy - Ay);

    if((Cy - By) != 0)
    {
        for(i32 Y = By; Y <= Cy; ++Y)
        {
            i32 StartX = Bx + InvSlope1*(Y - By);
            i32 EndX   = Ax + InvSlope2*(Y - Ay);

            if(EndX < StartX)
            {
                i32 Temp = StartX;
                StartX = EndX;
                EndX = Temp;
            }

            for(i32 X = StartX; X <= EndX; ++X)
            {
                DrawTrianglePixel(X, Y, A, B, C, Color);
            }
        }
    }
}

static void
DrawTexel(i32 X, i32 Y, v4 A, v4 B, v4 C, 
          tex2d AUVs, tex2d BUVs, tex2d CUVs, 
          upng_t* Texture)
{
    v2 P = V2(X, Y);
    v3 Bary = GetBarycentricWeights(A.XY, B.XY, C.XY, P); // NOTE: Alpha, Beta, Gamma

    v3 TextCoords = {Bary.X*AUVs.u/A.W + Bary.Y*BUVs.u/B.W + Bary.Z*CUVs.u/C.W, // NOTE: Interpolated U
                     Bary.X*AUVs.v/A.W + Bary.Y*BUVs.v/B.W + Bary.Z*CUVs.v/C.W, // NOTE: Interpolated V
                     Bary.X*(1/A.W) + Bary.Y*(1/B.W) + Bary.Z*(1/C.W)}; // NOTE: Interpolated Reciprocal W

    if(TextCoords.Z != 0)
    {
        TextCoords.X /= TextCoords.Z;
        TextCoords.Y /= TextCoords.Z;
    }
    
    i32 TextX = (i32)AbsoluteValue((i32)(TextCoords.X * Texture->width))  % Texture->width;
    i32 TextY = (i32)AbsoluteValue((i32)(TextCoords.Y * Texture->height)) % Texture->height;

    TextCoords.Z = 1.0f - TextCoords.Z;

    if (TextCoords.Z < GetZBufferValue(X, Y))
    {
        u32* TextureBuffer = (u32*)upng_get_buffer(Texture);
        DrawPixel(X, Y, TextureBuffer[TextY * Texture->width + TextX]);
        PutZBufferPixel(X, Y, TextCoords.Z);
    }
}

void 
DrawTexturedTriangle(triangle_t Triangle, upng_t* Texture)
{
    v4 A = Triangle.points[0];
    v4 B = Triangle.points[1];
    v4 C = Triangle.points[2];

    tex2d AUVs = Triangle.TextureCoords[0];
    tex2d BUVs = Triangle.TextureCoords[1];
    tex2d CUVs = Triangle.TextureCoords[2];

    if(A.Y > B.Y) {SwapTwo_v4(&A, &B);SwapTwo_tex2d(&AUVs, &BUVs);}
    if(B.Y > C.Y) {SwapTwo_v4(&B, &C);SwapTwo_tex2d(&BUVs, &CUVs);}
    if(A.Y > B.Y) {SwapTwo_v4(&A, &B);SwapTwo_tex2d(&AUVs, &BUVs);}

    AUVs.v = 1 - AUVs.v;
    BUVs.v = 1 - BUVs.v;
    CUVs.v = 1 - CUVs.v;

    int Ax = A.X;
    int Ay = A.Y;
    int Bx = B.X;
    int By = B.Y;
    int Cx = C.X;
    int Cy = C.Y;

    r32 InvSlope1 = 0;
    r32 InvSlope2 = 0;

    if((By - Ay) != 0) InvSlope1 = (r32)(Bx - Ax) / AbsoluteValue(By - Ay);
    if((Cy - Ay) != 0) InvSlope2 = (r32)(Cx - Ax) / AbsoluteValue(Cy - Ay);

    if((By - Ay) != 0)
    {
        for(i32 Y = Ay; Y <= By; ++Y)
        {
            i32 StartX = Bx + InvSlope1*(Y - By);
            i32 EndX   = Ax + InvSlope2*(Y - Ay);

            if(EndX < StartX)
            {
                i32 Temp = StartX;
                StartX = EndX;
                EndX = Temp;
            }

            for(i32 X = StartX; X < EndX; X++)
            {
                DrawTexel(X, Y, A, B, C, AUVs, BUVs, CUVs, Texture);
            }
        }
    }

    InvSlope1 = 0;
    InvSlope2 = 0;

    if((Cy - By) != 0) InvSlope1 = (r32)(Cx - Bx) / AbsoluteValue(Cy - By);
    if((Cy - Ay) != 0) InvSlope2 = (r32)(Cx - Ax) / AbsoluteValue(Cy - Ay);

    if((Cy - By) != 0)
    {
        for(i32 Y = By; Y <= Cy; ++Y)
        {
            i32 StartX = Bx + InvSlope1*(Y - By);
            i32 EndX   = Ax + InvSlope2*(Y - Ay);

            if(EndX < StartX)
            {
                i32 Temp = StartX;
                StartX = EndX;
                EndX = Temp;
            }

            for(i32 X = StartX; X < EndX; X++)
            {
                DrawTexel(X, Y, A, B, C, AUVs, BUVs, CUVs, Texture);
            }
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

// TODO: Needed further optimisations
// TODO: This routine have to use 4x block to work better
void DrawFilledTriangleFast(triangle_t Triangle, uint32_t Color)
{
    __m128i WindowWidth  = _mm_set1_epi32(GetWindowWidth());
    __m128i WindowHeight = _mm_set1_epi32(GetWindowHeight());
    __m128i Zero = _mm_set1_epi32(0.0f);
    __m128i One  = _mm_set1_epi32(1.0f);
    __m128 Half = _mm_set1_ps(0.5f);

    __m128i Ax  = _mm_set1_epi32(Triangle.points[0].X);
    __m128i Ay  = _mm_set1_epi32(Triangle.points[0].Y);
    __m128i Bx  = _mm_set1_epi32(Triangle.points[1].X);
    __m128i By  = _mm_set1_epi32(Triangle.points[1].Y);
    __m128i Cx  = _mm_set1_epi32(Triangle.points[2].X);
    __m128i Cy  = _mm_set1_epi32(Triangle.points[2].Y);

    __m128i cmp0 = _mm_cmpgt_epi32(Ay, By);
    if(_mm_movemask_epi8(cmp0))
    {
        Ax = _mm_xor_si128(Ax, Bx);
        Bx = _mm_xor_si128(Bx, Ax);
        Ax = _mm_xor_si128(Ax, Bx);

        Ay = _mm_xor_si128(Ay, By);
        By = _mm_xor_si128(By, Ay);
        Ay = _mm_xor_si128(Ay, By);
    } // SwapVectors(&A, &B);
    __m128i cmp1 = _mm_cmpgt_epi32(By, Cy);
    if(_mm_movemask_epi8(cmp1))
    {
        Bx = _mm_xor_si128(Bx, Cx);
        Cx = _mm_xor_si128(Cx, Bx);
        Bx = _mm_xor_si128(Bx, Cx);
                             
        By = _mm_xor_si128(By, Cy);
        Cy = _mm_xor_si128(Cy, By);
        By = _mm_xor_si128(By, Cy);
    } // SwapVectors(&B, &C);
    cmp0 = _mm_cmpgt_epi32(Ay, By);
    if(_mm_movemask_epi8(cmp0))
    {
        Ax = _mm_xor_si128(Ax, Bx);
        Bx = _mm_xor_si128(Bx, Ax);
        Ax = _mm_xor_si128(Ax, Bx);
                             
        Ay = _mm_xor_si128(Ay, By);
        By = _mm_xor_si128(By, Ay);
        Ay = _mm_xor_si128(Ay, By);
    } // SwapVectors(&A, &B);

    cmp0 = _mm_cmpeq_epi32(Ay, By);
    cmp1 = _mm_cmpeq_epi32(By, Cy);
    if(_mm_movemask_epi8(cmp1)) 
    {
#if 1
        __m128 Slope1 = _mm_div_ps(_mm_cvtepi32_ps(_mm_sub_epi32(Bx, Ax)), _mm_cvtepi32_ps(_mm_sub_epi32(By, Ay)));
        __m128 Slope2 = _mm_div_ps(_mm_cvtepi32_ps(_mm_sub_epi32(Cx, Ax)), _mm_cvtepi32_ps(_mm_sub_epi32(Cy, Ay)));

        __m128 StartX = _mm_cvtepi32_ps(Ax);
        __m128 EndX   = _mm_cvtepi32_ps(Ax);

        for(i32 Y = ((i32*)&Ay)[0]; Y <= ((i32*)&Cy)[0]; Y++)
        {
            __m128i MinX = _mm_cvtps_epi32(StartX);
            __m128i MinY = _mm_set1_epi32(Y);
            __m128i MaxX = _mm_cvtps_epi32(EndX);
            __m128i MaxY = _mm_set1_epi32(Y);

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

            // DrawPixel(V2(StartX, Y), V2(EndX, Y), Color)
            for(i32 PointIdx = 0; PointIdx <= ((i32*)&SideLength)[0]; PointIdx++)
            {
                DrawPixelFast(CurrentX, CurrentY, Color);

                CurrentX = _mm_add_ps(CurrentX, IncX);
                CurrentY = _mm_add_ps(CurrentY, IncY);
            }

            StartX = _mm_add_ps(StartX, Slope1);
            EndX   = _mm_add_ps(EndX,   Slope2);
        }
#endif
    } // FillFlatBottomTriangle(A, B, C, Color);
    else if(_mm_movemask_epi8(cmp0))
    {
        __m128 Slope1 = _mm_div_ps(_mm_cvtepi32_ps(_mm_sub_epi32(Cx, Ax)), _mm_cvtepi32_ps(_mm_sub_epi32(Cy, Ay)));
        __m128 Slope2 = _mm_div_ps(_mm_cvtepi32_ps(_mm_sub_epi32(Cx, Bx)), _mm_cvtepi32_ps(_mm_sub_epi32(Cy, By)));

        __m128 StartX = _mm_cvtepi32_ps(Cx);
        __m128 EndX   = _mm_cvtepi32_ps(Cx);

        for(i32 Y = ((i32*)&Cy)[0]; Y >= ((i32*)&Ay)[0]; --Y)
        {
            __m128i MinX = _mm_cvtps_epi32(StartX);
            __m128i MinY = _mm_set1_epi32(Y);
            __m128i MaxX = _mm_cvtps_epi32(EndX);
            __m128i MaxY = _mm_set1_epi32(Y);

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
                DrawPixelFast(CurrentX, CurrentY, Color);

                CurrentX = _mm_add_ps(CurrentX, IncX);
                CurrentY = _mm_add_ps(CurrentY, IncY);
            }

            StartX = _mm_sub_ps(StartX, Slope1);
            EndX   = _mm_sub_ps(EndX,   Slope2);
        }
    } // FillFlatTopTriangle(A, B, C, Color);
    else 
    {
        __m128 Adx = _mm_cvtepi32_ps(Ax);
        __m128 Ady = _mm_cvtepi32_ps(Ay);
        __m128 Bdx = _mm_cvtepi32_ps(Bx);
        __m128 Bdy = _mm_cvtepi32_ps(By);
        __m128 Cdx = _mm_cvtepi32_ps(Cx);
        __m128 Cdy = _mm_cvtepi32_ps(Cy);

        // Find Triangle Mid Point
        //v2 M = FindTriangleMid(A, B, C);
        __m128i My = By;
        __m128i Temp0 = _mm_cvtps_epi32(_mm_div_ps(_mm_mul_ps(_mm_sub_ps(Bdy, Ady), _mm_sub_ps(Cdx, Adx)), _mm_sub_ps(Cdy, Ady)));
        __m128i Mx = _mm_add_epi32(Ax, Temp0);

        // Flat-bottom triangle
        // A = A, B = B, C = M
        __m128 Slope1 = _mm_div_ps(_mm_cvtepi32_ps(_mm_sub_epi32(Bx, Ax)), _mm_cvtepi32_ps(_mm_sub_epi32(By, Ay)));
        __m128 Slope2 = _mm_div_ps(_mm_cvtepi32_ps(_mm_sub_epi32(Mx, Ax)), _mm_cvtepi32_ps(_mm_sub_epi32(My, Ay)));

        __m128 StartX = _mm_cvtepi32_ps(Ax);
        __m128 EndX   = _mm_cvtepi32_ps(Ax);

        for(i32 Y = ((i32*)&Ay)[0]; Y <= ((i32*)&My)[0]; Y++)
        {
            __m128i MinX = _mm_cvtps_epi32(StartX);
            __m128i MinY = _mm_set1_epi32(Y);
            __m128i MaxX = _mm_cvtps_epi32(EndX);
            __m128i MaxY = _mm_set1_epi32(Y);

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

            // DrawPixel(V2(StartX, Y), V2(EndX, Y), Color)
            for(i32 PointIdx = 0; PointIdx <= ((i32*)&SideLength)[0]; PointIdx++)
            {
                DrawPixelFast(CurrentX, CurrentY, Color);

                CurrentX = _mm_add_ps(CurrentX, IncX);
                CurrentY = _mm_add_ps(CurrentY, IncY);
            }

            StartX = _mm_add_ps(StartX, Slope1);
            EndX   = _mm_add_ps(EndX,   Slope2);
        }

        // Flat-top triangle
        // A = B, B = M, C = C
        Slope1 = _mm_div_ps(_mm_cvtepi32_ps(_mm_sub_epi32(Cx, Bx)), _mm_cvtepi32_ps(_mm_sub_epi32(Cy, By)));
        Slope2 = _mm_div_ps(_mm_cvtepi32_ps(_mm_sub_epi32(Cx, Mx)), _mm_cvtepi32_ps(_mm_sub_epi32(Cy, My)));

        StartX = _mm_cvtepi32_ps(Cx);
        EndX   = _mm_cvtepi32_ps(Cx);

        for(i32 Y = ((i32*)&Cy)[0]; Y >= ((i32*)&By)[0]; --Y)
        {
            __m128i MinX = _mm_cvtps_epi32(StartX);
            __m128i MinY = _mm_set1_epi32(Y);
            __m128i MaxX = _mm_cvtps_epi32(EndX);
            __m128i MaxY = _mm_set1_epi32(Y);

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
                DrawPixelFast(CurrentX, CurrentY, Color);

                CurrentX = _mm_add_ps(CurrentX, IncX);
                CurrentY = _mm_add_ps(CurrentY, IncY);
            }

            StartX = _mm_sub_ps(StartX, Slope1);
            EndX   = _mm_sub_ps(EndX,   Slope2);
        }
    }
}

