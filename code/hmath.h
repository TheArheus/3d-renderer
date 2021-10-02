#if !defined(HMATH_H_)
#define HMATH_H_

union v2
{
    struct
    {
        r32 X, Y;        
    };
    r32 E[2];
};

union v3
{
    struct
    {
        r32 X, Y, Z;        
    };
    struct
    {
        r32 R, G, B;
    };
    struct
    {
        v2 XY;
        r32 Ignored0_;
    };
    struct
    {
        r32 Ignored1_;
        v2 YZ;
    };
    struct
    {
        v2 RG;
        r32 Ignored0_;
    };
    struct
    {
        r32 Ignored1_;
        v2 GB;
    };
    r32 E[3];
};

union v4
{
    struct
    {
        r32 X, Y, Z, W;
    };
    struct
    {
        r32 R, G, B, A;
    };
    struct
    {
        v2 XY;
        r32 Ignored0_;
        r32 Ignored1_;
    };
    struct
    {
        r32 Ignored2_;
        v2 YZ;
        r32 Ignored3_;
    };
    struct
    {
        r32 Ignored4_;
        r32 Ignored5_;
        v2 ZW;
    };
    struct
    {
        v3 XYZ;
        r32 Ignored6_;
    };
    struct
    {
        v3 RGB;
        r32 Ignored6_;
    };
    r32 E[4];
};

inline v2
V2(r32 X, r32 Y)
{
    v2 Result = {};

    Result.X = X;
    Result.Y = Y;

    return Result;
}

inline v3
V3(r32 X, r32 Y, r32 Z)
{
    v3 Result = {};

    Result.X = X;
    Result.Y = Y;
    Result.Z = Z;

    return Result;
}

inline v3
V3(v2 A, r32 Z)
{
    v3 Result = {};

    Result.X = A.X;
    Result.Y = A.Y;
    Result.Z = Z;

    return Result;
}

inline v3
V3(r32 X, v2 A)
{
    v3 Result = {};

    Result.X = X;
    Result.Y = A.X;
    Result.Z = A.Y;

    return Result;
}

inline v4
V4(r32 X, r32 Y, r32 Z, r32 W)
{
    v4 Result = {};

    Result.X = X;
    Result.Y = Y;
    Result.Z = Z;
    Result.W = W;

    return Result;
}

inline v4
ToV4(v3 XYZ, r32 W)
{
    v4 Result = {};

    Result.XYZ = XYZ;
    Result.W = W;

    return Result;
}

// Basing vector operations
//
//
// 2-D Vector operations
//

inline v2 
operator+(v2 A, v2 B)
{
    v2 Result = {};

    Result.X = A.X + B.X;
    Result.Y = A.Y + B.Y;

    return Result;
}

inline v2
operator+(v2 A, r32 B)
{
    v2 Result = {};

    Result.X = A.X + B;
    Result.Y = A.Y + B;

    return Result;
}

inline v2
operator+(r32 A, v2 B)
{
    B = B + A;

    return B;
}

inline v2&
operator+=(v2& A, v2 B)
{
    A = A + B;

    return A;
}

inline v2&
operator+=(v2& A, r32 B)
{
    A = A + B;

    return A;
}

inline v2 
operator-(v2 A, v2 B)
{
    v2 Result = {};

    Result.X = A.X - B.X;
    Result.Y = A.Y - B.Y;

    return Result;
}

inline v2
operator-(v2 A, r32 B)
{
    v2 Result = {};

    Result.X = A.X - B;
    Result.Y = A.Y - B;

    return Result;
}

inline v2
operator-(r32 A, v2 B)
{
    B = B - A;

    return B;
}

inline v2&
operator-=(v2& A, v2 B)
{
    A = A - B;

    return A;
}

inline v2&
operator-=(v2& A, r32 B)
{
    A = A - B;

    return A;
}

inline v2
operator*(v2 A, v2 B)
{
    v2 Result = {};

    Result.X = A.X*B.X;
    Result.Y = A.Y*B.Y;

    return Result;
}

inline v2
operator*(v2 A, r32 B)
{
    v2 Result = {};

    Result.X = A.X*B;
    Result.Y = A.Y*B;

    return Result;
}

inline v2
operator*(r32 A, v2 B)
{
    B = B*A;

    return B;
}

inline v2&
operator*=(v2& A, v2 B)
{
    A = A*B;

    return A;
}

inline v2&
operator*=(v2& A, r32 B)
{
    A = A * B;

    return A;
}

//
// 3-D Vector operations
//

inline v3 
operator+(v3 A, v3 B)
{
    v3 Result = {};

    Result.X = A.X + B.X;
    Result.Y = A.Y + B.Y;
    Result.Z = A.Z + B.Z;

    return Result;
}

inline v3
operator+(v3 A, r32 B)
{
    v3 Result = {};

    Result.X = A.X + B;
    Result.Y = A.Y + B;
    Result.Z = A.Z + B;

    return Result;
}

inline v3
operator+(r32 A, v3 B)
{
    B = B + A;

    return B;
}

inline v3&
operator+=(v3& A, v3 B)
{
    A = A + B;

    return A;
}

inline v3&
operator+=(v3& A, r32 B)
{
    A = A + B;

    return A;
}

inline v3 
operator-(v3 A, v3 B)
{
    v3 Result = {};

    Result.X = A.X - B.X;
    Result.Y = A.Y - B.Y;
    Result.Z = A.Z - B.Z;

    return Result;
}

inline v3 
operator-(v3 A)
{
    v3 Result = {};

    Result.X = -A.X;
    Result.Y = -A.Y;
    Result.Z = -A.Z;

    return Result;
}

inline v3
operator-(v3 A, r32 B)
{
    v3 Result = {};

    Result.X = A.X - B;
    Result.Y = A.Y - B;
    Result.Z = A.Z - B;

    return Result;
}

inline v3
operator-(r32 A, v3 B)
{
    B = B - A;

    return B;
}

inline v3&
operator-=(v3& A, v3 B)
{
    A = A - B;

    return A;
}

inline v3&
operator-=(v3& A, r32 B)
{
    A = A - B;

    return A;
}

inline v3
operator*(v3 A, v3 B)
{
    v3 Result = {};

    Result.X = A.X*B.X;
    Result.Y = A.Y*B.Y;
    Result.Z = A.Z*B.Z;

    return Result;
}

inline v3
operator*(v3 A, r32 B)
{
    v3 Result = {};

    Result.X = A.X*B;
    Result.Y = A.Y*B;
    Result.Z = A.Z*B;

    return Result;
}

inline v3
operator*(r32 A, v3 B)
{
    B = B*A;

    return B;
}

inline v3&
operator*=(v3& A, v3 B)
{
    A = A*B;

    return A;
}

inline v3&
operator*=(v3& A, r32 B)
{
    A = A * B;

    return A;
}

//
// 4-D Vector operations
//

inline v4 
operator+(v4 A, v4 B)
{
    v4 Result = {};

    Result.X = A.X + B.X;
    Result.Y = A.Y + B.Y;
    Result.Z = A.Z + B.Z;
    Result.W = A.W + B.W;

    return Result;
}

inline v4
operator+(v4 A, r32 B)
{
    v4 Result = {};

    Result.X = A.X + B;
    Result.Y = A.Y + B;
    Result.Z = A.Z + B;

    return Result;
}

inline v4
operator+(r32 A, v4 B)
{
    B = B + A;

    return B;
}

inline v4&
operator+=(v4& A, v4 B)
{
    A = A + B;

    return A;
}

inline v4&
operator+=(v4& A, r32 B)
{
    A = A + B;

    return A;
}

inline v4 
operator-(v4 A, v4 B)
{
    v4 Result = {};

    Result.X = A.X - B.X;
    Result.Y = A.Y - B.Y;
    Result.Z = A.Z - B.Z;
    Result.W = A.W - B.W;

    return Result;
}

inline v4
operator-(v4 A, r32 B)
{
    v4 Result = {};

    Result.X = A.X - B;
    Result.Y = A.Y - B;
    Result.Z = A.Z - B;

    return Result;
}

inline v4
operator-(r32 A, v4 B)
{
    B = B - A;

    return B;
}

inline v4&
operator-=(v4& A, v4 B)
{
    A = A - B;

    return A;
}

inline v4&
operator-=(v4& A, r32 B)
{
    A = A - B;

    return A;
}

inline v4
operator*(v4 A, v4 B)
{
    v4 Result = {};

    Result.X = A.X*B.X;
    Result.Y = A.Y*B.Y;
    Result.Z = A.Z*B.Z;
    Result.W = A.W*B.W;

    return Result;
}

inline v4
operator*(v4 A, r32 B)
{
    v4 Result = {};

    Result.X = A.X*B;
    Result.Y = A.Y*B;
    Result.Z = A.Z*B;
    Result.W = A.W*B;

    return Result;
}

inline v4
operator*(r32 A, v4 B)
{
    B = B*A;

    return B;
}

inline v4&
operator*=(v4& A, v4 B)
{
    A = A*B;

    return A;
}

inline v4&
operator*=(v4& A, r32 B)
{
    A = A * B;

    return A;
}

//
// Other operations on vectors
//

inline v3
rotate_x(v3 P, r32 A)
{
    v3 Result = {};

    Result.X = P.X;
    Result.Y = P.Y*cosf(A) - P.Z*sinf(A);
    Result.Z = P.Y*sinf(A) + P.Z*cosf(A);

    return Result;
}

inline v3
rotate_y(v3 P, r32 A)
{
    v3 Result = {};

    Result.X = P.X*cosf(A) - P.Z*sinf(A);
    Result.Y = P.Y;
    Result.Z = P.X*sinf(A) + P.Z*cosf(A);

    return Result;
}

inline v3 
rotate_z(v3 P, r32 A)
{
    v3 Result = {};

    Result.X = P.X*cosf(A) - P.Y*sinf(A);
    Result.Y = P.X*sinf(A) + P.Y*cosf(A);
    Result.Z = P.Z;

    return Result;
}

inline v3
Cross(v3 A, v3 B)
{
    v3 Result = {};

    Result.X = A.Y*B.Z - A.Z*B.Y;
    Result.Y = A.Z*B.X - A.X*B.Z;
    Result.Z = A.X*B.Y - A.Y*B.X;

    return Result;
}

inline r32
AbsoluteValue(r32 A)
{
    r32 Result = fabs(A);
    return Result;
}

inline r32
SquareRoot(r32 A)
{
    r32 Result = sqrtf(A);
    return Result;
}

inline r32
Inner(v2 A, v2 B) // Dot product
{
    return A.X*B.X + A.Y*B.Y;
}

inline r32
Inner(v3 A, v3 B)
{
    return A.X*B.X + A.Y*B.Y + A.Z*B.Z;
}

inline r32
LengthSq(v2 A)
{
    return Inner(A, A);
}

inline r32
LengthSq(v3 A)
{
    return Inner(A, A);
}

inline r32
Length(v2 A)
{
    return SquareRoot(LengthSq(A));
}

inline r32
Length(v3 A)
{
    return SquareRoot(LengthSq(A));
}

inline r32
Clamp(r32 V, r32 Min, r32 Max)
{
    r32 Result = V;
    if(V < Min){ Result = Min; }
    if(V > Max){ Result = Max; }
    return Result;
}

inline r32
Clamp01(r32 V)
{
    r32 Result = Clamp(V, 0.0f, 1.0f);
    return Result;
}

inline v3
Clamp(v3 V, r32 Min, r32 Max)
{
    v3 Result = V;
    if((V.X < Min) && (V.Y < Min) && (V.Z < Min)){ Result = V3(Min, Min, Min); }
    if((V.X > Max) && (V.Y > Max) && (V.Z > Max)){ Result = V3(Max, Max, Max); }
    return Result;
}

inline v3
Clamp01(v3 V)
{
    v3 Result = Clamp(V, 0.0f, 1.0f);
    return Result;
}

inline v4
Clamp(v4 V, r32 Min, r32 Max)
{
    v4 Result = V;
    if((V.X < Min) && (V.Y < Min) && (V.Z < Min) && (V.W < Min)){ Result = V4(Min, Min, Min, Min); }
    if((V.X > Max) && (V.Y > Max) && (V.Z > Max) && (V.W > Max)){ Result = V4(Max, Max, Max, Max); }
    return Result;
}

inline v4
Clamp01(v4 V)
{
    v4 Result = Clamp(V, 0.0f, 1.0f);
    return Result;
}

#endif
