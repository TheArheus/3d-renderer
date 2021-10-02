

inline m4x4 operator*(m4x4 A, m4x4 B)
{
    m4x4 Result = {};

    for(int r = 0; r <= 3; ++r) // Rows of A
    {
        for(int c = 0; c <= 3; ++c) // Cols of B
        {
            for(int i = 0; i <= 3; ++i) // Cols of A, Rows of B
            {
                Result.E[r][c] += A.E[r][i]*B.E[i][c];
            }
        }
    }

    return Result;
}

v3 Transform(m4x4 A, v3 P, r32 Pw)
{
    v3 V = {};

    V.X = P.X*A.E[0][0] + P.Y*A.E[0][1] + P.Z*A.E[0][2] + Pw*A.E[0][3];
    V.Y = P.X*A.E[1][0] + P.Y*A.E[1][1] + P.Z*A.E[1][2] + Pw*A.E[1][3];
    V.Z = P.X*A.E[2][0] + P.Y*A.E[2][1] + P.Z*A.E[2][2] + Pw*A.E[2][3];

    return V;
}

v4 Transform(m4x4 A, v4 P)
{
    v4 V = {};

    V.X = P.X*A.E[0][0] + P.Y*A.E[0][1] + P.Z*A.E[0][2] + P.W*A.E[0][3];
    V.Y = P.X*A.E[1][0] + P.Y*A.E[1][1] + P.Z*A.E[1][2] + P.W*A.E[1][3];
    V.Z = P.X*A.E[2][0] + P.Y*A.E[2][1] + P.Z*A.E[2][2] + P.W*A.E[2][3];
    V.W = P.X*A.E[3][0] + P.Y*A.E[3][1] + P.Z*A.E[3][2] + P.W*A.E[3][3];

    return V;
}

inline v3
operator*(m4x4 A, v3 P)
{
    v3 R = Transform(A, P, 1.0f);
    return R;
}

inline v4
operator*(m4x4 A, v4 P)
{
    v4 R = Transform(A, P);
    return R;
}

m4x4 GetIdentity()
{
    m4x4 Identity = 
    {
        {{1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1}}
    };

    return Identity;
}

m4x4 MakeScaleMatrix(r32 X, r32 Y, r32 Z)
{
    m4x4 Scale = GetIdentity();

    Scale.E[0][0] = X;
    Scale.E[1][1] = Y;
    Scale.E[2][2] = Z;

    return Scale;
}

m4x4 Translate(m4x4 A, v3 P)
{
    m4x4 R = A;

    R.E[0][3] += P.X;
    R.E[1][3] += P.Y;
    R.E[2][3] += P.Z;

    return R;
}

m4x4 MakePerspectiveProjection(r32 FOV, r32 Aspect, r32 ZNear, r32 ZFar)
{
    m4x4 Result = {{{0}}};

    Result.E[0][0] = Aspect * (1 / tanf(FOV / 2));
    Result.E[1][1] = (1 / tanf(FOV / 2));
    Result.E[2][2] = ZFar / (ZFar - ZNear);
    Result.E[2][3] = (-ZFar*ZNear)/(ZFar - ZNear);
    Result.E[3][2] = 1.0f;

    return Result;
}

v4 Project(m4x4 Proj, v4 V)
{
    v4 Result = Proj * V;

    if(Result.W != 0.0)
    {
        Result.X /= Result.W;
        Result.Y /= Result.W;
        Result.Z /= Result.W;
    }

    return Result;
}

m4x4 RotateX(r32 V)
{
    r32 c = sinf(V);
    r32 s = cosf(V);

    m4x4 R = 
    {
        {{1, 0,  0, 0},
         {0, c, -s, 0},
         {0, s,  c, 0},
         {0, 0,  0, 1}}
    };

    return R;
}

m4x4 RotateY(r32 V)
{
    r32 c = sinf(V);
    r32 s = cosf(V);

    m4x4 R = 
    {
        {{c, 0, -s, 0},
         {0, 1,  0, 0},
         {s, 0,  c, 0},
         {0, 0,  0, 1}}
    };

    return R;
}

m4x4 RotateZ(r32 V)
{
    r32 c = sinf(V);
    r32 s = cosf(V);

    m4x4 R = 
    {
        {{c, -s, 0, 0},
         {s,  c, 0, 0},
         {0,  0, 1, 0},
         {0,  0, 0, 1}}
    };

    return R;
}
