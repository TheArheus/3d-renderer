
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


mesh_t Mesh = {};

v3 CubeVertices[N_CUBE_VERTICES] = 
{
    {-1, -1, -1},
    {-1,  1, -1},
    { 1,  1, -1},
    { 1, -1, -1},
    { 1,  1,  1},
    { 1, -1,  1},
    {-1,  1,  1},
    {-1, -1,  1},
};

face_t CubeMeshes[N_CUBE_MESHES] = 
{
    // front
    {1, 2, 3, 0xFFFFFFFF},
    {1, 3, 4, 0xFFFFFFFF},
    // right
    {4, 3, 5, 0xFFFFFFFF},
    {4, 5, 6, 0xFFFFFFFF},
    // back
    {6, 5, 7, 0xFFFFFFFF},
    {6, 7, 8, 0xFFFFFFFF},
    // left
    {8, 7, 2, 0xFFFFFFFF},
    {8, 2, 1, 0xFFFFFFFF},
    // top
    {2, 7, 5, 0xFFFFFFFF},
    {2, 5, 3, 0xFFFFFFFF},
    // bottom
    {6, 8, 1, 0xFFFFFFFF},
    {6, 1, 4, 0xFFFFFFFF},
};

struct vertex_indices 
{
    face_t Face;
};

void LoadCubeMeshData(void)
{
    for(u32 VertexIdx = 0; VertexIdx < N_CUBE_VERTICES; ++VertexIdx)
    {
        v3 CubeVertex = CubeVertices[VertexIdx];
        ArrayPush(Mesh.Vertices, CubeVertex, v3);
    }

    for(u32 MeshIdx = 0; MeshIdx < N_CUBE_MESHES; ++MeshIdx)
    {
        face_t CubeMesh = CubeMeshes[MeshIdx];
        ArrayPush(Mesh.Meshes, CubeMesh, face_t);
    }
}


static v3
ParseVertex(char* String)
{
    v3 Result = {};

    char ValueToConvert[10] = {0};
    int ValueIdx = 0;
    int VertIdx = 0;
    for(char* StringToConv = String; *StringToConv;)
    {
        if(*StringToConv == ' ' || *StringToConv == '\n')
        {
            Result.E[VertIdx] = strtof(ValueToConvert, NULL);
            memset(ValueToConvert, 0, sizeof(ValueToConvert));

            ValueIdx = 0;
            VertIdx++;
            StringToConv++;
            continue;
        }

        ValueToConvert[ValueIdx++] = *StringToConv;
        StringToConv++;
    }
    
    return Result;
}

static face_t
ParseFace(char* String)
{
    vertex_indices Result = {};

    char ValueToConvert[8] = {0};
    int ValueIdx = 0;
    int MeshIdx = 0;
    int Type = 0;
    for(char* StringToConv = String; *StringToConv;)
    {
        if (*StringToConv == ' ' || *StringToConv == '\n')
        {
            if(Type == 0)
            {
                Result.Face.E[MeshIdx++] = atoi(ValueToConvert);
                memset(ValueToConvert, 0, sizeof(ValueToConvert));
            }

            ValueIdx = 0;
            Type = 0;
            StringToConv++;
            continue;
        }
        else if(*StringToConv == '/')
        {
            if(Type == 0)
            {
                Result.Face.E[MeshIdx++] = atoi(ValueToConvert);
                memset(ValueToConvert, 0, sizeof(ValueToConvert));
            }

            Type = (Type + 1) % 3;
            StringToConv++;
            continue;
        }

        if(Type == 0)
        {
            ValueToConvert[ValueIdx++] = *StringToConv;
            StringToConv++;
        }
        else
        {
            StringToConv++;
        }
    }

    return Result.Face;
}

static void
ParseObjLine(char* Line)
{
    char PrevChar = 0;
    char Char = 0;
    while(Char = *Line++)
    {
        if(Char == '#' || Char == 'm' || Char == 'o' && Char == 'u' || Char == 's') break;
        if((PrevChar == 'v' && Char == 't')) break;
        if((PrevChar == 'v' && Char == 'n')) break;

        if((PrevChar == 'v' && Char == ' '))
        {
            v3 Vert = ParseVertex(Line);

            ArrayPush(Mesh.Vertices, Vert, v3);
            break;
        } 
        if((PrevChar == 'f' && Char == ' '))
        {
            face_t Face = ParseFace(Line);

            Face.Color = 0xFFFFFFFF;
            ArrayPush(Mesh.Meshes, Face, face_t);
            break;
        }
        PrevChar = Char;
    }
}

void LoadObjFileData(char* FileName)
{
    FILE* file = NULL;
    fopen_s(&file, FileName, "r");
    
    char Line[256];
    while(fgets(Line, 256, (FILE*)file) != NULL)
    {
        ParseObjLine(Line);
    }
}
