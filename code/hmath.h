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

#endif
