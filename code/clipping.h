#if !defined(CLIPPING_H_)
#define CLIPPING_H_

#define MAX_POLY_VERTICES_COUNT  128
#define MAX_POLY_TRIANGLES_COUNT 128

enum
{
    Plane_Left,
    Plane_Right,
    Plane_Top,
    Plane_Bottom,
    Plane_Near,
    Plane_Far,
    Plane_Count,
};

struct plane_t
{
    v3 Point;
    v3 Normal;
};

struct polygon_t
{
    v3 Vertices[MAX_POLY_VERTICES_COUNT];
    tex2d TextureCoords[MAX_POLY_VERTICES_COUNT];
    u32 VerticesCount;
};

static polygon_t CreatePolygon(v3 A, v3 B, v3 C, tex2d A_uv, tex2d B_uv, tex2d C_uv);
static void InitializeFrustrumPlanes(r32 FOVx, r32 FOVy, r32 NearZ, r32 FarZ);
static void ClipPolygon(polygon_t* Polygon);
static triangle_t* PolygonTriangulate(polygon_t* Polygon, i32* TrianglesCount);
static void TrianglesFromPolygon(polygon_t* Polygon, triangle_t Triangles[], i32* TrianglesCount);

extern plane_t Planes[Plane_Count];

#endif
