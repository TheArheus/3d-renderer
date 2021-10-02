#if !defined(MATRIX_H_)
#define MATRIX_H_

struct m4x4
{
    r32 E[4][4];
};

m4x4 GetIdentity(void);
m4x4 MakeScaleMatrix(r32 X, r32 Y, r32 Z);
v3 Transform(m4x4 A, v3 P, r32 Pw);
m4x4 MakePerspectiveProjection(r32 FOV, r32 Aspect, r32 ZNear, r32 ZFar);
v4 Project(m4x4 Proj, v4 V);

#endif
