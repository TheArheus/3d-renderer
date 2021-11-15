#if !defined(TEXTURE_H_)
#define TEXTURE_H_

#include "hmath.h"

union tex2d
{
    struct
    {
        r32 u, v;
    };
    r32 E[2];
};


static tex2d TexCoordsCopy(tex2d* T);
static tex2d V3ToUVs(v3 Vert);
static tex2d V2ToUVs(v2 Vert);


//
// Intrinsics 
//

inline tex2d 
operator+(tex2d A, tex2d B)
{
    tex2d Result = {};

    Result.u = A.u + B.u;
    Result.v = A.v + B.v;

    return Result;
}

inline tex2d
operator+(tex2d A, r32 B)
{
    tex2d Result = {};

    Result.u = A.u + B;
    Result.v = A.v + B;

    return Result;
}

inline tex2d
operator+(r32 A, tex2d B)
{
    B = B + A;

    return B;
}

inline tex2d 
operator-(tex2d A, tex2d B)
{
    tex2d Result = {};

    Result.u = A.u - B.u;
    Result.v = A.v - B.v;

    return Result;
}

inline tex2d
operator-(tex2d A, r32 B)
{
    tex2d Result = {};

    Result.u = A.u - B;
    Result.v = A.v - B;

    return Result;
}

inline tex2d
operator-(r32 A, tex2d B)
{
    B = B - A;

    return B;
}

inline tex2d
operator*(tex2d A, tex2d B)
{
    tex2d Result = {};

    Result.u = A.u*B.u;
    Result.v = A.v*B.v;

    return Result;
}

inline tex2d
operator*(tex2d A, r32 B)
{
    tex2d Result = {};

    Result.u = A.u*B;
    Result.v = A.v*B;

    return Result;
}

inline tex2d
operator*(r32 A, tex2d B)
{
    B = B*A;

    return B;
}

inline tex2d
Lerp(tex2d A, r32 t, tex2d B)
{
    tex2d Result = (1 - t)*A + t*B;

    return Result;
}

#endif
