

static tex2d 
V3ToUVs(v3 Vert)
{
    tex2d Result = {};
    Result.u = Vert.X;
    Result.v = Vert.Y;
    return Result;
}

static tex2d 
V2ToUVs(v2 Vert)
{
    tex2d Result = {};
    Result.u = Vert.X;
    Result.v = Vert.Y;
    return Result;
}

static tex2d
TexCoordsCopy(tex2d* T)
{
    tex2d Result = {};

    Result.u = T->u;
    Result.v = T->v;

    return Result;
}
