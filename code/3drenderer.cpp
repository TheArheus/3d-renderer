#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#include "upng.c"
#include "display.h"
#include "texture.cpp"
#include "mesh.cpp"
#include "array.cpp"
#include "triangle.cpp"
#include "matrix.cpp"
#include "Camera.cpp"
#include "clipping.cpp"

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

#define TRIANGLES_PER_MESH (1 << 13)
triangle_t TrianglesToRender[TRIANGLES_PER_MESH];
i32 TrianglesToRenderCount = 0;

r32 DeltaTime = 0;
i32 PreviousFrameTime = 0;
u32 DisplayFlag = DisplayType_Lines | DisplayType_Culling;

lighting Light;
m4x4 ViewMatrix;
m4x4 ProjectionMatrix;
m4x4 WorldMatrix;

static void 
setup(void)
{
    Light.Direction = V3(0, 0, 1);

    r32 AspectRatioY = (r32)GetWindowHeight() / (r32)GetWindowWidth();
    r32 AspectRatioX = (r32)GetWindowWidth()  / (r32)GetWindowHeight();
    r32 FOVy = M_PI / 3.0f;
    r32 FOVx = atanf(tanf(FOVy / 2) * AspectRatioX) * 2;
    r32 Near =  1.0f;
    r32 Far  = 50.0f;
    ProjectionMatrix = MakePerspectiveProjection(FOVy, AspectRatioY, Near, Far);

    InitializeFrustrumPlanes(FOVx, FOVy, Near, Far);

    LoadMesh("../data/runway.obj", "../data/runway.png", V3(1, 1, 1), V3(0, -3.5f, 23), V3(0, 0, 0));
    LoadMesh("../data/f22.obj",  "../data/f22.png",  V3(1, 1, 1), V3( 6, -3.3f, -2), V3(0, -M_PI/2, 0));
    LoadMesh("../data/f117.obj", "../data/f117.png", V3(1, 1, 1), V3(10, -3.3f,  0), V3(0, -M_PI/2, 0));
    LoadMesh("../data/efa.obj", "../data/efa.png",   V3(1, 1, 1), V3( 6, -3.3f,  2), V3(0, -M_PI/2, 0));
}

static void 
process_input(void)
{
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
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
                    else if(event.key.keysym.sym == SDLK_5) DisplayFlag ^= DisplayType_Textured;
                    else if(event.key.keysym.sym == SDLK_4) DisplayFlag ^= (DisplayType_Lines | DisplayType_FilledTriangles);
                    else if(event.key.keysym.sym == SDLK_c) DisplayFlag |= DisplayType_Culling;
                    else if(event.key.keysym.sym == SDLK_x) DisplayFlag &= ~DisplayType_Culling;
                    else if(event.key.keysym.sym == SDLK_w) Camera.Pitch += 3.0f * DeltaTime;
                    else if(event.key.keysym.sym == SDLK_s) Camera.Pitch -= 3.0f * DeltaTime;
                    else if(event.key.keysym.sym == SDLK_LEFT)  Camera.Yaw -= 1.0f * DeltaTime;
                    else if(event.key.keysym.sym == SDLK_RIGHT) Camera.Yaw += 1.0f * DeltaTime;
                    else if(event.key.keysym.sym == SDLK_UP)   
                    {
                        Camera.ForwardSpeed = Camera.Direction * (5.0f * DeltaTime); 
                        Camera.Position += Camera.ForwardSpeed;
                    }
                    else if(event.key.keysym.sym == SDLK_DOWN) 
                    {
                        Camera.ForwardSpeed = Camera.Direction * (5.0f * DeltaTime); 
                        Camera.Position -= Camera.ForwardSpeed;
                    }
                }
                break;
        }
    }
}


static void
ProcessGraphicsPipeline(mesh_t* Mesh)
{
    m4x4 ScaleMatrix = MakeScaleMatrix(Mesh->Scale.X, Mesh->Scale.Y, Mesh->Scale.Z);
    m4x4 TranslationMatrix = Translate(GetIdentity(), Mesh->Translation);
    m4x4 RotationX = RotateX(Mesh->Rotation.X);
    m4x4 RotationY = RotateY(Mesh->Rotation.Y);
    m4x4 RotationZ = RotateZ(Mesh->Rotation.Z);

    v3 Target = V3(0, 0, 1);

    m4x4 CameraYawRotation   = RotateY(Camera.Yaw);
    m4x4 CameraPitchRotation = RotateX(Camera.Pitch);

    m4x4 CameraRotation = GetIdentity();
    CameraRotation = CameraPitchRotation * CameraRotation;
    CameraRotation = CameraYawRotation   * CameraRotation;
    Camera.Direction = CameraRotation * Target;

    Target     = Camera.Position + Camera.Direction;
    v3 UpDir   = V3(0, 1, 0);
    ViewMatrix = LookAt(Camera.Position, Target, UpDir);

    u32 FaceCount = ArrayLength(Mesh->Meshes);
    for(u32 FaceIndex = 0; 
        FaceIndex < FaceCount; 
        ++FaceIndex)
    {
        face_t Face = Mesh->Meshes[FaceIndex];

        v3 DispatchedVertices[3];
        DispatchedVertices[0] = Mesh->Vertices[Face.a - 1];
        DispatchedVertices[1] = Mesh->Vertices[Face.b - 1];
        DispatchedVertices[2] = Mesh->Vertices[Face.c - 1];

        v4 TransfermedVertices[3] = {0};

        for(u32 PointIndex = 0; 
            PointIndex < 3; 
            ++PointIndex)
        {
            v4 Point = V4(DispatchedVertices[PointIndex], 1.0f);

            WorldMatrix = GetIdentity();

            WorldMatrix = WorldMatrix*ScaleMatrix;
            WorldMatrix = WorldMatrix*RotationX;
            WorldMatrix = WorldMatrix*RotationY;
            WorldMatrix = WorldMatrix*RotationZ;
            WorldMatrix = WorldMatrix*TranslationMatrix;
            
            Point = WorldMatrix*Point;
            Point = ViewMatrix *Point;

            TransfermedVertices[PointIndex] = Point;
        }

        // Check backface culling (Culling algorithm)
        v3 FaceNormal = GetTriangleNormal(TransfermedVertices);

        if(DisplayFlag & DisplayType_Culling)
        {
            v3 CameraRay = V3(0, 0, 0) - TransfermedVertices[0].XYZ;
            r32 Angle = Inner(FaceNormal, CameraRay);

            // Bypass the triangles that are looking away from the camera
            if(Angle < 0)
            {
                continue;
            }
        }

        polygon_t Polygon = CreatePolygon(TransfermedVertices[0].XYZ, 
                                          TransfermedVertices[1].XYZ, 
                                          TransfermedVertices[2].XYZ, 
                                          Face.UVs.A_uv, Face.UVs.B_uv, Face.UVs.C_uv);
        ClipPolygon(&Polygon);
        
        i32 TrianglesCount = 0;
        triangle_t TriangulateResult[MAX_POLY_TRIANGLES_COUNT] = {};
        TrianglesFromPolygon(&Polygon, TriangulateResult, &TrianglesCount);

        //triangle_t* TriangulatingResult = PolygonTriangulate(&Polygon, &TrianglesCount);

        for(i32 TriangleIndex = 0;
            TriangleIndex < TrianglesCount;
            ++TriangleIndex)
        {
            triangle_t* ClippedTriangle = TriangulateResult + TriangleIndex;

            v4 ProjectedPoints[3] = {};
            for (u32 PointIdx = 0;
                PointIdx < 3;
                ++PointIdx)
            {
                ProjectedPoints[PointIdx] = Project(ProjectionMatrix, ClippedTriangle->points[PointIdx]);
                
                ProjectedPoints[PointIdx].Y  *= -1.0f;
                ProjectedPoints[PointIdx].XY *= V2(GetWindowWidth() / 2.0f, GetWindowHeight() / 2.0f);
                ProjectedPoints[PointIdx].XY += V2(GetWindowWidth() / 2.0f, GetWindowHeight() / 2.0f);
            }

            r32 LightIntensity = -Inner(FaceNormal, Light.Direction);
            u32 NewColor = ApplyLightIntensity(Face.Color, LightIntensity);

            triangle_t TriangleToRender = {};
            TriangleToRender.Color = NewColor;
            TriangleToRender.Texture = Mesh->Texture;
            memcpy(TriangleToRender.points, ProjectedPoints, sizeof(v4) * 3);
            memcpy(TriangleToRender.TextureCoords, ClippedTriangle->TextureCoords, sizeof(tex2d) * 3);

            if (TrianglesToRenderCount < TRIANGLES_PER_MESH)
            {
                TrianglesToRender[TrianglesToRenderCount++] = TriangleToRender;
            }
        }
    }
}

static void 
update(void)
{
    int TimeToWait = FRAME_TARGET_TIME - (SDL_GetTicks() - PreviousFrameTime);

    if((TimeToWait > 0) && (TimeToWait <= FRAME_TARGET_TIME))
    {
        SDL_Delay(TimeToWait);
    }

    DeltaTime = (SDL_GetTicks() - PreviousFrameTime) / 1000.0f;

    PreviousFrameTime = SDL_GetTicks();

    TrianglesToRenderCount = 0;

    for(i32 MeshIndex = 0;
        MeshIndex < MeshesCount;
        ++MeshIndex)
    {
        mesh_t* Mesh = Meshes + MeshIndex;
        ProcessGraphicsPipeline(Mesh);
    }
}

static void 
render(void)
{
    ClearColorBuffer(CreateColor(V4(0.0f, 0.0f, 0.0f, 255.0f)));
    ClearZBuffer();

    DrawGrid(CreateColor(V4(12.75f, 12.75f, 12.75f, 255.0f)));

    for(i32 CoordIdx = 0; 
        CoordIdx < TrianglesToRenderCount; 
        ++CoordIdx)
    {
        triangle_t TriangleToRender = TrianglesToRender[CoordIdx];

        if(DisplayFlag & DisplayType_FilledTriangles)
        {
            //DrawFilledTriangleFast(TriangleToRender, TriangleToRender.Color);
            DrawFilledTriangle(TriangleToRender, TriangleToRender.Color);
        }
        if(DisplayFlag & DisplayType_Textured)
        {
            DrawTexturedTriangle(TriangleToRender, TriangleToRender.Texture);
        }
        if(DisplayFlag & DisplayType_Lines)
        {
            DrawTriangle(TriangleToRender, TriangleToRender.Color);
        }
        if(DisplayFlag & DisplayType_Dots)
        {
            for(u32 PointIdx = 0; PointIdx < 3; ++PointIdx)
            {
                DrawRect(TriangleToRender.points[PointIdx].XY - 3, 6, 6, CreateColorN(V4(1.0f, 1.0f, 1.0f, 1.0f)));
            }
        }
    }
    
#if 0
    v2 Center = V2(0.5f*GetWindowWidth(), 0.5f*GetWindowHeight());

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
    
    for(u32 CoordIdx = 1; 
        CoordIdx < Shape->VerticesCount + 1; 
        ++CoordIdx)
    {
        DrawLine(20*Shape->Vertices[(CoordIdx - 1) % Shape->VerticesCount].XY + Center, 20*Shape->Vertices[CoordIdx % Shape->VerticesCount].XY + Center, CreateColor(V3(1.0f, 1.0f, 1.0f)));
    }

    for(u32 CoordIdx = 0; 
        CoordIdx < Shape->TrianglesCount; 
        CoordIdx += 3)
    {
        triangle_t Triangle = {};

        Triangle.points[0] = 20*Shape->Vertices[Shape->Triangles[CoordIdx  ]].XY + Center;
        Triangle.points[1] = 20*Shape->Vertices[Shape->Triangles[CoordIdx+1]].XY + Center;
        Triangle.points[2] = 20*Shape->Vertices[Shape->Triangles[CoordIdx+2]].XY + Center;

        DrawTriangle(Triangle, CreateColor(V3(1.0f, 1.0f, 0.0f)));
    }
#endif

    RenderColorBuffer();
}

static void
FreeResources(void)
{
    for(i32 MeshIndex = 0;
        MeshIndex < MeshesCount;
        ++MeshIndex)
    {
        mesh_t* Mesh = Meshes + MeshIndex;
        ArrayFree(Mesh->Meshes);
        ArrayFree(Mesh->Vertices);
        upng_free(Mesh->Texture);
    }
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
