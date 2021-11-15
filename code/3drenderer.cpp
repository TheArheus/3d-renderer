#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#include "display.h"
#include "mesh.cpp"
#include "array.cpp"
#include "triangle.cpp"
#include "matrix.cpp"

static u32 
CreateColorN(v4 Color)
{
    Color = Clamp01(Color);
    u32 Result = ((u32)roundf(255.0f*Color.A) << 24 |
                  (u32)roundf(255.0f*Color.R) << 16 | 
                  (u32)roundf(255.0f*Color.G) <<  8 | 
                  (u32)roundf(255.0f*Color.B) <<  0);

    return Result;
}

static v4
ExtractColorN(u32 Color)
{
    v4 Result = V4(((Color >> 16) & 0xFF) / 255.0f, 
                   ((Color >>  8) & 0xFF) / 255.0f, 
                   ((Color >>  0) & 0xFF) / 255.0f, 
                   ((Color >> 24) & 0xFF) / 255.0f);
    return Result;
}

static u32 
CreateColor(v4 Color)
{
    Color = Clamp(Color, 0.0f, 255.0f);
    u32 Result = ((u32)roundf(Color.A) << 24 |
                  (u32)roundf(Color.R) << 16 | 
                  (u32)roundf(Color.G) <<  8 | 
                  (u32)roundf(Color.B) <<  0);

    return Result;
}

static v4
ExtractColor(u32 Color)
{
    v4 Result = V4(((Color >> 16) & 0xFF), 
                   ((Color >>  8) & 0xFF), 
                   ((Color >>  0) & 0xFF), 
                   ((Color >> 24) & 0xFF));
    return Result;
}

static u32
ApplyLightIntensity(u32 Color, r32 Intensity)
{
    Intensity = Clamp01(Intensity);
    u32 A = (Color & 0xFF000000);
    u32 R = (Color & 0x00FF0000) * Intensity;
    u32 G = (Color & 0x0000FF00) * Intensity; 
    u32 B = (Color & 0x000000FF) * Intensity; 

    u32 Result = ((A) | (R & 0x00FF0000) | (G & 0x0000FF00) | (B & 0x000000FF));

    return Result;
}

struct lighting
{
    v3 Direction;
};

bool is_running;

triangle_t* TrianglesToRender = NULL;
v3 CameraPos = {0, 0, 0};

i32 PreviousFrameTime = 0;
u32 DisplayFlag = DisplayType_FilledTriangles | DisplayType_Lines | DisplayType_Culling;

lighting Light;
m4x4 ProjectionMatrix;

//
// Test Triangulation method
//

struct shape
{
    u32 VerticesCount;
    v3* Vertices;

    u32 TrianglesCount;
    int* Triangles;
};

static bool IsPointInTriangle(v3 Point, v3 A, v3 B, v3 C)
{
    v3 AB = B - A;
    v3 BC = C - B;
    v3 CA = A - C;

    v3 AP = Point - A;
    v3 BP = Point - B;
    v3 CP = Point - C;

    float Cross1 = Cross(AB, AP).Z;
    float Cross2 = Cross(BC, BP).Z;
    float Cross3 = Cross(CA, CP).Z;

    if((Cross1 > 0.0f) || (Cross2 > 0.0f) || (Cross3 > 0.0f))
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

static void PolygonTriangulate(shape* Shape)
{
    bool Result = false;

    int  ListSize  = Shape->VerticesCount;
    int* IndexList = (int*)malloc(sizeof(int) * ListSize);
    for(int i = 0; i < ListSize; ++i)
    {
        IndexList[i] = i;
    }

    int TotalTriangleCount = Shape->VerticesCount - 2;
    int TotalTriangleCountIdx = TotalTriangleCount*3;

    int* TrianglesResult = (int*)malloc(sizeof(int)*TotalTriangleCountIdx);
    int  TriangleIdx = 0;

    while(ListSize > 3)
    {
        for(int PolyIdx = 0; PolyIdx < ListSize; ++PolyIdx)
        {
            int a = IndexList[PolyIdx];
            int b = GetListElement(IndexList, ListSize, PolyIdx - 1);
            int c = GetListElement(IndexList, ListSize, PolyIdx + 1);

            v3 A = Shape->Vertices[a];
            v3 B = Shape->Vertices[b];
            v3 C = Shape->Vertices[c];

            v3 AB = B - A;
            v3 AC = C - A;
            
            if(Cross(AB, AC).Z < 0.0f)
            {
                continue;
            }

            bool IsEar = true;

            for(int i = 0; i < ListSize; ++i)
            {
                if((a == i) || (b == i) || (c == i))
                {
                    continue;
                }

                v3 Point = Shape->Vertices[i];
                
                if(IsPointInTriangle(Point, B, A, C))
                {
                    IsEar = false;
                    break;
                }
            }

            if(IsEar)
            {
                TrianglesResult[TriangleIdx++] = b;
                TrianglesResult[TriangleIdx++] = a;
                TrianglesResult[TriangleIdx++] = c;

                RemoveElementFromList(IndexList, &ListSize, PolyIdx);
                break;
            }
        }
    }

    TrianglesResult[TriangleIdx++] = IndexList[0];
    TrianglesResult[TriangleIdx++] = IndexList[1];
    TrianglesResult[TriangleIdx++] = IndexList[2];

    Shape->TrianglesCount = TriangleIdx;
    Shape->Triangles = TrianglesResult;
}
//
//
//

static void 
setup(void)
{
    Mesh.Vertices    = NULL;
    Mesh.Meshes      = NULL;
    Mesh.Rotation    = V3(0, 0, 0);
    Mesh.Scale       = V3(1, 1, 1);
    Mesh.Translation = V3(0, 0, 0);

    color_buffer = (u32*)malloc(sizeof(u32)*window_width*window_height);

    Light.Direction = V3(0, 0, 1);
    //Light.Direction = Light.Direction * (1.0f / Length(Light.Direction));
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, 
                                SDL_TEXTUREACCESS_STREAMING, 
                                window_width, window_height);

    r32 FOV = M_PI / 3.0f;
    r32 AspectRatio = (r32)window_height / (r32)window_width;
    r32 Near = 0.1;
    r32 Far  = 100.0;
    ProjectionMatrix = MakePerspectiveProjection(FOV, AspectRatio, Near, Far);

    //LoadCubeMeshData();
    LoadObjFileData("../data/f22.obj");
}

static void 
process_input(void)
{
    SDL_Event event;
    SDL_PollEvent(&event);

    switch(event.type)
    {
        case SDL_QUIT:
            is_running = false;
            break;
        case SDL_KEYDOWN:
            {
                if(event.key.keysym.sym == SDLK_ESCAPE) is_running = false;
                else if(event.key.keysym.sym == SDLK_1) DisplayFlag ^= DisplayType_Dots;
                else if(event.key.keysym.sym == SDLK_2) DisplayFlag ^= DisplayType_Lines;
                else if(event.key.keysym.sym == SDLK_3) DisplayFlag ^= DisplayType_FilledTriangles;
                else if(event.key.keysym.sym == SDLK_4) DisplayFlag ^= (DisplayType_Lines | DisplayType_FilledTriangles);
                else if(event.key.keysym.sym == SDLK_c) DisplayFlag |= DisplayType_Culling;
                else if(event.key.keysym.sym == SDLK_d) DisplayFlag &= ~DisplayType_Culling;
            }
            break;
    }
}

static void
swap(triangle_t* Triangles, u32 left, u32 right)
{
    triangle_t temp = Triangles[left];
    Triangles[left] = Triangles[right];
    Triangles[right] = temp;
}

static void
Sort(triangle_t* Triangles, u32 Size)
{
    if (Size == 0) return;
    bool IsSorted = false;
    u32 LastUnsorted = Size - 1;
    while(!IsSorted)
    {
        IsSorted = true;
        for(u32 Idx = 0; Idx < LastUnsorted; ++Idx)
        {
            if(Triangles[Idx].AvgDepth < Triangles[Idx + 1].AvgDepth)
            {
                swap(Triangles, Idx, Idx + 1);
                IsSorted = false;
            }
        }
        LastUnsorted--;
    }
}

static void 
update(void)
{
    int TimeToWait = FRAME_TARGET_TIME - (SDL_GetTicks() + PreviousFrameTime);

    TrianglesToRender = NULL;

    if(TimeToWait > 0 && (TimeToWait <= FRAME_TARGET_TIME))
    {
        SDL_Delay(TimeToWait);
    }

    Mesh.Rotation += 0.002f;
    Mesh.Translation.Z = 5.0f;

    m4x4 ScaleMatrix = MakeScaleMatrix(Mesh.Scale.X, Mesh.Scale.Y, Mesh.Scale.Z);
    m4x4 TranslationMatrix = Translate(GetIdentity(), Mesh.Translation);
    m4x4 RotationX = RotateX(Mesh.Rotation.X);
    m4x4 RotationY = RotateY(Mesh.Rotation.Y);
    m4x4 RotationZ = RotateZ(Mesh.Rotation.Z);

    u32 MeshesCount = ArrayLength(Mesh.Meshes);
    for(u32 MeshIdx = 0; MeshIdx < MeshesCount; ++MeshIdx)
    {
        face_t Face = Mesh.Meshes[MeshIdx];

        v3 DispatchedVertices[3];
        DispatchedVertices[0] = Mesh.Vertices[Face.a - 1];
        DispatchedVertices[1] = Mesh.Vertices[Face.b - 1];
        DispatchedVertices[2] = Mesh.Vertices[Face.c - 1];

        triangle_t NewPointToRender;
        v3 TransfermedVertices[3] = {0};
        for(u32 VertIdx = 0; VertIdx < 3; ++VertIdx)
        {
            v3 Point = DispatchedVertices[VertIdx];

            m4x4 WorldMatrix = GetIdentity();

            WorldMatrix = WorldMatrix*TranslationMatrix;
            WorldMatrix = WorldMatrix*RotationX;
            WorldMatrix = WorldMatrix*RotationY;
            WorldMatrix = WorldMatrix*RotationZ;
            WorldMatrix = WorldMatrix*ScaleMatrix;
            
            Point = WorldMatrix*Point;

            TransfermedVertices[VertIdx] = Point;
        }

        // Check backface culling (Culling algorithm)
        v3 A = TransfermedVertices[0];
        v3 B = TransfermedVertices[1];
        v3 C = TransfermedVertices[2];

        v3 AB = B - A; // B - A
        v3 AC = C - A; // C - A
        AB *= (1.0f / Length(AB));
        AC *= (1.0f / Length(AC));
        v3 Normal = Cross(AB, AC);
        Normal *= (1.0f / Length(Normal));

        if(DisplayFlag & DisplayType_Culling)
        {
            v3 CameraRay = CameraPos - A;
            r32 Angle = Inner(Normal, CameraRay);

            // Bypass the triangles that are looking away from the camera
            if(Angle < 0)
            {
                continue;
            }
        }

        for(u32 VertIdx = 0; VertIdx < 3; ++VertIdx)
        {
            v3 NewPoint = Project(ProjectionMatrix, ToV4(TransfermedVertices[VertIdx], 0)).XYZ;

            NewPoint *= V3(window_width / 2.0f, window_height / 2.0f, 1.0f);
            NewPoint += V3((r32)window_width/2, (r32)window_height/2, 0.0f);

            NewPointToRender.points[VertIdx] = NewPoint.XY;
        }

        v3 Centroid = (TransfermedVertices[0] + TransfermedVertices[1] + TransfermedVertices[2])* (1.0f/3);

        v3 LightRay = Light.Direction;
        r32 LightIntensity = -Inner(Normal, LightRay);
        u32 NewColor = ApplyLightIntensity(Face.Color, LightIntensity);

        NewPointToRender.AvgDepth = Centroid.Z;
        NewPointToRender.Color = NewColor;

        ArrayPush(TrianglesToRender, NewPointToRender, triangle_t);
    }

    Sort(TrianglesToRender, ArrayLength(TrianglesToRender));
}

static void 
render(void)
{
    DrawGrid(CreateColor(V4(12.75f, 12.75f, 12.75f, 255.0f)));

    
    u32 TrianglesCount = ArrayLength(TrianglesToRender);
    for(u32 CoordIdx = 0; CoordIdx < TrianglesCount; ++CoordIdx)
    {
        triangle_t TriangleToRender = TrianglesToRender[CoordIdx];

        if(DisplayFlag & DisplayType_FilledTriangles)
        {
            DrawFilledTriangleFast(TriangleToRender, TriangleToRender.Color);
        }
        if(DisplayFlag & DisplayType_Lines)
        {
            DrawTriangleFast(TriangleToRender, TriangleToRender.Color);
        }
        if(DisplayFlag & DisplayType_Dots)
        {
            for(u32 PointIdx = 0; PointIdx < 3; ++PointIdx)
            {
                DrawRect(TriangleToRender.points[PointIdx], 4, 4, CreateColorN(V4(1.0f, 1.0f, 1.0f, 1.0f)));
            }
        }
    }
    
    /*
    v2 Center = V2(0.5f*window_width, 0.5f*window_height);

    shape* Shape = (shape*)malloc(sizeof(shape));
    Shape->VerticesCount = 9;
    Shape->Vertices = (v3*)malloc(sizeof(v3)*Shape->VerticesCount);
    Shape->Vertices[0] = V3(-4,  6, 0.0f);
    Shape->Vertices[1] = V3( 0,  2, 0.0f);
    Shape->Vertices[2] = V3( 2,  5, 0.0f);
    Shape->Vertices[3] = V3( 7,  0, 0.0f);
    Shape->Vertices[4] = V3( 5, -6, 0.0f);
    Shape->Vertices[5] = V3( 3,  3, 0.0f);
    Shape->Vertices[6] = V3( 0, -5, 0.0f);
    Shape->Vertices[7] = V3(-6,  0, 0.0f);
    Shape->Vertices[8] = V3(-2,  1, 0.0f);

    PolygonTriangulate(Shape);
    
    for(u32 CoordIdx = 1; CoordIdx < Shape->VerticesCount + 1; ++CoordIdx)
    {
        DrawLine(20*Shape->Vertices[(CoordIdx - 1) % Shape->VerticesCount].XY + Center, 20*Shape->Vertices[CoordIdx % Shape->VerticesCount].XY + Center, CreateColor(V3(1.0f, 1.0f, 1.0f)));
    }

    for(u32 CoordIdx = 0; CoordIdx < Shape->TrianglesCount; CoordIdx += 3)
    {
        triangle_t Triangle = {};

        Triangle.points[0] = 20*Shape->Vertices[Shape->Triangles[CoordIdx  ]].XY + Center;
        Triangle.points[1] = 20*Shape->Vertices[Shape->Triangles[CoordIdx+1]].XY + Center;
        Triangle.points[2] = 20*Shape->Vertices[Shape->Triangles[CoordIdx+2]].XY + Center;

        DrawTriangle(Triangle, CreateColor(V3(1.0f, 1.0f, 0.0f)));
    }
    */

    ArrayFree(TrianglesToRender);

    RenderColorBuffer();
    ClearColorBuffer(CreateColor(V4(0.0f, 0.0f, 0.0f, 255.0f)));

    SDL_RenderPresent(renderer);
}

static void
FreeResources(void)
{
    free(color_buffer);
    ArrayFree(Mesh.Meshes);
    ArrayFree(Mesh.Vertices);
}

int main(int argc, char** argv)
{
    is_running = InitWindow();

    setup();

    while(is_running)
    {
        process_input();
        update();
        render();
    }

    DestroyWindow();
    FreeResources();

    return 0;
}
