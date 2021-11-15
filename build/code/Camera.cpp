
camera_t Camera = 
{
    {0, 0, 0},
    {0, 0, 1},
    {0, 0, 0},
     0,
     0,
};

m4x4 LookAt(v3 Eye, v3 Target, v3 Up)
{
    v3 Z = Target - Eye;
    Z /= Length(Z);
    v3 X = Cross(Up, Z);
    X /= Length(X);
    v3 Y = Cross(Z, X);

    m4x4 ViewMatrix =
    {
        {{X.X, X.Y, X.Z, -Inner(X, Eye)},
         {Y.X, Y.Y, Y.Z, -Inner(Y, Eye)},
         {Z.X, Z.Y, Z.Z, -Inner(Z, Eye)},
         {  0,   0,   0,             1}}
    };
    return ViewMatrix;
}

