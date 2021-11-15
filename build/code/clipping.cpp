
plane_t Planes[Plane_Count] = {};

static void
InitializeFrustrumPlanes(r32 FOVx, r32 FOVy, r32 NearZ, r32 FarZ)
{
    r32 Cx = cosf(FOVx/2);
    r32 Sx = sinf(FOVx/2);
    r32 Cy = cosf(FOVy/2);
    r32 Sy = sinf(FOVy/2);

    Planes[Plane_Left].Point  = V3(0.0f);
    Planes[Plane_Left].Normal = {Cx, 0, Sx};

    Planes[Plane_Right].Point  = V3(0.0f);
    Planes[Plane_Right].Normal = {-Cx, 0, Sx};

    Planes[Plane_Top].Point  = V3(0.0f);
    Planes[Plane_Top].Normal = {0, -Cy, Sy};

    Planes[Plane_Bottom].Point  = V3(0.0f);
    Planes[Plane_Bottom].Normal = {0, Cy, Sy};

    Planes[Plane_Near].Point  = V3(0, 0, NearZ);
    Planes[Plane_Near].Normal = {0, 0, 1};

    Planes[Plane_Far].Point  = V3(0, 0, FarZ);
    Planes[Plane_Far].Normal = {0, 0, -1};
}


static polygon_t
CreatePolygon(v3 A, v3 B, v3 C, tex2d A_uv, tex2d B_uv, tex2d C_uv)
{
    polygon_t Polygon = {};

    Polygon.VerticesCount = 3;

    Polygon.Vertices[0] = A;
    Polygon.Vertices[1] = B;
    Polygon.Vertices[2] = C;

    Polygon.TextureCoords[0] = A_uv;
    Polygon.TextureCoords[1] = B_uv;
    Polygon.TextureCoords[2] = C_uv;

    return Polygon;
}

static void
ClipPolygon(polygon_t* Polygon, plane_t* Plane)
{
    if(Polygon->VerticesCount != 0)
    {
        v3 PlanePoint = Plane->Point;
        v3 PlaneNormal = Plane->Normal;

        v3 InsideVertices[MAX_POLY_VERTICES_COUNT] = {};
        tex2d InsideTexCoords[MAX_POLY_VERTICES_COUNT] = {};
        i32 InsideVerticesCount = 0;

        v3* CurrentVertex = &Polygon->Vertices[0];
        tex2d* CurrentTextureCoord = &Polygon->TextureCoords[0];

        v3* PreviousVertex = &Polygon->Vertices[Polygon->VerticesCount - 1];
        tex2d* PreviousTextureCoord = &Polygon->TextureCoords[Polygon->VerticesCount - 1];

        r32 CurrentDot  = 0;
        r32 PreviousDot = Inner(*PreviousVertex - PlanePoint, PlaneNormal);

        while (CurrentVertex != &Polygon->Vertices[Polygon->VerticesCount])
        {
            CurrentDot = Inner(*CurrentVertex - PlanePoint, PlaneNormal);

            if (CurrentDot * PreviousDot < 0)
            {
                r32 t = PreviousDot / (PreviousDot - CurrentDot);

                v3 IntersectionPoint = Lerp(V3Copy(PreviousVertex), t, V3Copy(CurrentVertex));
                tex2d IntersectionTextureCoord = Lerp(TexCoordsCopy(PreviousTextureCoord), t, TexCoordsCopy(CurrentTextureCoord));

                InsideVertices[InsideVerticesCount] = IntersectionPoint;
                InsideTexCoords[InsideVerticesCount] = IntersectionTextureCoord;
                InsideVerticesCount++;
            }

            if (CurrentDot > 0)
            {
                InsideVertices[InsideVerticesCount]  = V3Copy(CurrentVertex);
                InsideTexCoords[InsideVerticesCount] = TexCoordsCopy(CurrentTextureCoord);

                InsideVerticesCount++;
            }

            PreviousDot = CurrentDot;
            PreviousVertex = CurrentVertex;
            PreviousTextureCoord = CurrentTextureCoord;

            CurrentTextureCoord++;
            CurrentVertex++;
        }

        for (i32 VertIndex = 0;
            VertIndex < InsideVerticesCount;
            ++VertIndex)
        {
            Polygon->Vertices[VertIndex] = V3Copy(&InsideVertices[VertIndex]);
            Polygon->TextureCoords[VertIndex] = TexCoordsCopy(&InsideTexCoords[VertIndex]);
        }
        Polygon->VerticesCount = InsideVerticesCount;
    }
}

static void
ClipPolygon(polygon_t* Polygon)
{
    ClipPolygon(Polygon, &Planes[Plane_Left]);
    ClipPolygon(Polygon, &Planes[Plane_Right]);
    ClipPolygon(Polygon, &Planes[Plane_Top]);
    ClipPolygon(Polygon, &Planes[Plane_Bottom]);
    ClipPolygon(Polygon, &Planes[Plane_Near]);
    ClipPolygon(Polygon, &Planes[Plane_Far]);
}

static void
TrianglesFromPolygon(polygon_t* Polygon, triangle_t Triangles[], i32* TrianglesCount)
{
    memset(Triangles, 0xFFFFFFFF, sizeof(Triangles));
    if(Polygon->VerticesCount > 0)
    {
        for (u32 VertIndex = 0;
            VertIndex < Polygon->VerticesCount - 2;
            ++VertIndex)
        {
            i32 a = 0;
            i32 b = VertIndex + 1;
            i32 c = VertIndex + 2;
            Triangles[VertIndex].points[0] = V4(Polygon->Vertices[a], 1.0f);
            Triangles[VertIndex].points[1] = V4(Polygon->Vertices[b], 1.0f);
            Triangles[VertIndex].points[2] = V4(Polygon->Vertices[c], 1.0f);

            Triangles[VertIndex].TextureCoords[0] = Polygon->TextureCoords[a];
            Triangles[VertIndex].TextureCoords[1] = Polygon->TextureCoords[b];
            Triangles[VertIndex].TextureCoords[2] = Polygon->TextureCoords[c];
        }

        *TrianglesCount = Polygon->VerticesCount - 2;
    }
}


//
///////////////////////////////////////////////////////////////////////////////////////
// Triangulation method
//

static bool 
IsPointInTriangle(v3 P, v3 A, v3 B, v3 C)
{
    v3 AB = B - A;
    v3 BC = C - B;
    v3 CA = A - C;

    v3 AP = P - A;
    v3 BP = P - B;
    v3 CP = P - C;

    float Cross1 = Cross(AB, AP).Z;
    float Cross2 = Cross(BC, BP).Z;
    float Cross3 = Cross(CA, CP).Z;

    if((Cross1 >= 0.0f) || (Cross2 >= 0.0f) || (Cross3 >= 0.0f))
    {
        return false;
    }
    return true;
}

int GetListElement(int* IndexList, int ListSize, int PolyIdx)
{
    int Result;

    if(PolyIdx < 0)
    {
        Result = IndexList[PolyIdx % ListSize + ListSize];
    }
    else if(PolyIdx >= ListSize)
    {
        Result = IndexList[PolyIdx % ListSize];
    }
    else
    {
        Result = IndexList[PolyIdx];
    }

    return Result;
}

void RemoveElementFromList(int* IndexList, int* ListSize, int PolyIdx)
{
    for(int i = 0; i < *ListSize; ++i)
    {
        if(i >= PolyIdx)
        {
            IndexList[i] = IndexList[i + 1];
        }
    }
    *ListSize -= 1;
}

static triangle_t* 
PolygonTriangulate(polygon_t* Polygon, i32* TrianglesCount)
{
    if(*TrianglesCount > 0)
    {
        int  ListSize = Polygon->VerticesCount;
        int* IndexList = (int*)malloc(sizeof(int) * ListSize);
        for (u32 Index = 0;
            Index < Polygon->VerticesCount;
            ++Index)
        {
            IndexList[Index] = Index;
        }

        i32 TotalTriangleCount = Polygon->VerticesCount - 2;
        //int TotalTriangleCountIdx = TotalTriangleCount*3;

        //int* TrianglesResult = (int*)malloc(sizeof(int)*TotalTriangleCountIdx);
        i32 TriangleIdx = 0;

        triangle_t* TrianglesResult = (triangle_t*)malloc(sizeof(triangle_t) * TotalTriangleCount);
        memset(TrianglesResult, 0, sizeof(triangle_t) * TotalTriangleCount);
        while (ListSize > 3)
        {
            for (int PolyIdx = 0; PolyIdx < ListSize; ++PolyIdx)
            {
                int a = IndexList[PolyIdx];
                int b = GetListElement(IndexList, ListSize, PolyIdx - 1);
                int c = GetListElement(IndexList, ListSize, PolyIdx + 1);

                v3 A = Polygon->Vertices[a];
                v3 B = Polygon->Vertices[b];
                v3 C = Polygon->Vertices[c];

                v3 AB = B - A;
                v3 AC = C - A;

                if (Cross(AB, AC).Z < 0.0f)
                {
                    continue;
                }

                bool IsEar = true;

                for (int i = 0; i < ListSize; ++i)
                {
                    if ((a == i) || (b == i) || (c == i))
                    {
                        continue;
                    }

                    v3 Point = Polygon->Vertices[i];

                    if (IsPointInTriangle(Point, B, A, C))
                    {
                        IsEar = false;
                        break;
                    }
                }

                if (IsEar)
                {
                    TrianglesResult[TriangleIdx++] = Triangle(B, A, C);
                    //TrianglesResult[TriangleIdx++] = b;
                    //TrianglesResult[TriangleIdx++] = a;
                    //TrianglesResult[TriangleIdx++] = c;

                    RemoveElementFromList(IndexList, &ListSize, PolyIdx);
                    break;
                }
            }
        }

        TrianglesResult[TriangleIdx++] = Triangle(Polygon->Vertices[IndexList[0]],
            Polygon->Vertices[IndexList[1]],
            Polygon->Vertices[IndexList[2]]);
        //TrianglesResult[TriangleIdx++] = IndexList[0];
        //TrianglesResult[TriangleIdx++] = IndexList[1];
        //TrianglesResult[TriangleIdx++] = IndexList[2];

        //Shape->TrianglesCount = TriangleIdx;
        //Shape->Triangles = TrianglesResult;
        free(IndexList);
        *TrianglesCount = TriangleIdx;
        return TrianglesResult;
    }
    return 0;
}
//
//////////////////////////////////////////////////////////////////////////////
//
