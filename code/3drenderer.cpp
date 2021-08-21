#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#include "display.h"
#include "mesh.cpp"
#include "array.cpp"
#include "triangle.cpp"

#undef main

static u32 
CreateColor(v3 Color)
{
    u32 Result = ((u32)roundf(Color.R * 255.0f) << 16 | 
                  (u32)roundf(Color.G * 255.0f) <<  8 | 
                  (u32)roundf(Color.B * 255.0f) <<  0);

    return Result;
}

bool is_running;

triangle_t* TrianglesToRender = NULL;
v3 CameraPos = {0, 0, 0};

i32 PreviousFrameTime = 0;
u32 DisplayFlag = DisplayType_FilledTriangles | DisplayType_Lines | DisplayType_Culling;

u32 CubeFOV = 700;

static void 
setup(void)
{
    color_buffer = (u32*)malloc(sizeof(u32)*window_width*window_height);

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, 
                                SDL_TEXTUREACCESS_STREAMING, 
                                window_width, window_height);

    // LoadCubeMeshData();
    LoadObjFileData("../data/cube.obj");
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

static v2 
project(v3 P)
{
    v2 Result;

    Result.X = (CubeFOV*P.X)/P.Z;
    Result.Y = (CubeFOV*P.Y)/P.Z;

    return Result;
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
    bool IsSorted = false;
    u32 LastUnsorted = Size - 1;
    while(!IsSorted)
    {
        IsSorted = true;
        for(u32 Idx = 0; Idx < LastUnsorted; ++Idx)
        {
            if(Triangles[Idx].AvgDepth > Triangles[Idx + 1].AvgDepth)
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

    Mesh.Rotation += 0.005;

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
            
            Point = rotate_x(Point, Mesh.Rotation.X);
            Point = rotate_y(Point, Mesh.Rotation.Y);
            Point = rotate_z(Point, Mesh.Rotation.Z);
            Point.Z -= 5;

            TransfermedVertices[VertIdx] = Point;
        }

        if(DisplayFlag & DisplayType_Culling)
        {
            // Check backface culling (Culling algorithm)
            v3 A = TransfermedVertices[0] * (1 / LengthSq(TransfermedVertices[0]));
            v3 B = TransfermedVertices[1] * (1 / LengthSq(TransfermedVertices[1]));
            v3 C = TransfermedVertices[2] * (1 / LengthSq(TransfermedVertices[2]));

            v3 BA = B - A; // B - A
            v3 CA = C - A; // C - A
            v3 Normal = Cross(BA, CA);

            v3 CameraRay = CameraPos - A;
            r32 angle = Inner(Normal, CameraRay);
            // Bypass the triangles that are looking away from the camera
            if(angle < 0)
            {
                continue;
            }
        }

        for(u32 VertIdx = 0; VertIdx < 3; ++VertIdx)
        {
            v2 NewPoint = project(TransfermedVertices[VertIdx]);
            NewPoint += V2((r32)window_width/2, (r32)window_height/2);

            NewPointToRender.points[VertIdx] = NewPoint;
        }
        r32 AvgDepth = (TransfermedVertices[0].Z + TransfermedVertices[1].Z + TransfermedVertices[2].Z) / 3;
        NewPointToRender.AvgDepth = AvgDepth;
        NewPointToRender.Color = Face.Color;

        ArrayPush(TrianglesToRender, NewPointToRender, triangle_t);
    }

    Sort(TrianglesToRender, ArrayLength(TrianglesToRender));
}

static void 
render(void)
{
    DrawGrid(CreateColor(V3(0.05f, 0.05f, 0.05f)));

    u32 TrianglesCount = ArrayLength(TrianglesToRender);
    for(u32 CoordIdx = 0; CoordIdx < TrianglesCount; ++CoordIdx)
    {
        triangle_t TriangleToRender = TrianglesToRender[CoordIdx];


        if(DisplayFlag & DisplayType_FilledTriangles)
        {
            DrawFilledTriangle(TriangleToRender, TriangleToRender.Color);
        }
        if(DisplayFlag & DisplayType_Lines)
        {
            DrawTriangle(TriangleToRender, CreateColor(V3(1.0f, 0.0f, 0.0f)));
        }
        if(DisplayFlag & DisplayType_Dots)
        {
            for(u32 PointIdx = 0; PointIdx < 3; ++PointIdx)
            {
                DrawRect(TriangleToRender.points[PointIdx], 4, 4, CreateColor(V3(1.0f, 1.0f, 1.0f)));
            }
        }
    }

    ArrayFree(TrianglesToRender);

    RenderColorBuffer();
    ClearColorBuffer(CreateColor(V3(0.0f, 0.0f, 0.0f)));

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
