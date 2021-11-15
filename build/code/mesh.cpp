
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

mesh_t Meshes[1 << 4] = {};
i32 MeshesCount = 0;

static v3
ParseVertex(char* String)
{
    v3 Result = {};

    char ValueToConvert[10] = {' '};
    char FirstChar = ' ';
    int ValueIdx = 0;
    int VertIdx = 0;
    for(char* StringToConv = String; *StringToConv;)
    {
        if (*StringToConv == 'v' || *StringToConv == 't') { FirstChar = *StringToConv++; continue; }
        if((*StringToConv == ' ' || *StringToConv == '\n') && (FirstChar != 'v') && (FirstChar != 't'))
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
        FirstChar = 0;
    }
    
    return Result;
}


struct parsed_obj 
{
    i32 VertexIndices[3];
    i32 TextureIndices[3];
    i32 NormalIndices[3];
};
static parsed_obj
ParseFace(char* String)
{
    parsed_obj Result = {};

    char ValueToConvert[8] = {' '};
    char FirstChar = 0;
    int  ValueIdx = 0;
    int  MeshIdx = 0;
    int  TextIdx = 0;
    int  NormIdx = 0;
    int  Type = 0;
    for(char* StringToConv = String; *StringToConv;)
    {
        if (*StringToConv == 'f') { FirstChar = *StringToConv++; continue; }
        if ((*StringToConv == ' ' || *StringToConv == '\n') && (FirstChar != 'f'))
        {
            if(ValueToConvert[0] != ' ')
            {
                if (Type == 0)
                {
                    Result.VertexIndices[MeshIdx++] = atoi(ValueToConvert);
                    memset(ValueToConvert, 0, sizeof(ValueToConvert));
                }
                if (Type == 1)
                {
                    Result.TextureIndices[TextIdx++] = atoi(ValueToConvert);
                    memset(ValueToConvert, 0, sizeof(ValueToConvert));
                }
                if (Type == 2)
                {
                    Result.NormalIndices[NormIdx++] = atoi(ValueToConvert);
                    memset(ValueToConvert, 0, sizeof(ValueToConvert));
                }
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
                Result.VertexIndices[MeshIdx++] = atoi(ValueToConvert);
                memset(ValueToConvert, 0, sizeof(ValueToConvert));
            }
            if(Type == 1)
            {
                Result.TextureIndices[TextIdx++] = atoi(ValueToConvert);
                memset(ValueToConvert, 0, sizeof(ValueToConvert));
            }
            if(Type == 2)
            {
                Result.NormalIndices[NormIdx++] = atoi(ValueToConvert);
                memset(ValueToConvert, 0, sizeof(ValueToConvert));
            }

            Type = (Type + 1) % 3;
            ValueIdx = 0;
            StringToConv++;
            continue;
        }
        
        if(FirstChar != 'f')
        {
            if (Type == 0)
            {
                ValueToConvert[ValueIdx++] = *StringToConv;
                StringToConv++;
            }
            else if (Type == 1)
            {
                ValueToConvert[ValueIdx++] = *StringToConv;
                StringToConv++;
            }
            else if (Type == 2)
            {
                ValueToConvert[ValueIdx++] = *StringToConv;
                StringToConv++;
            }
            else
            {
                StringToConv++;
            }
        }
        
        FirstChar = 0;
    }
    return Result;
}

void LoadMeshData(mesh_t* Mesh, char* FileName)
{
    FILE* file = NULL;
    fopen_s(&file, FileName, "r");
    
    tex2d* TextureCoords = NULL;
    char Line[256];
    while(fgets(Line, 256, (FILE*)file) != NULL)
    {
        i32 CharIdx = 0;
        char PrevChar = 0;
        char Char = Line[CharIdx];

        while(Char)
        {
            Char = Line[CharIdx++];
            if(Char == '#' || Char == 'm' || Char == 'o' && Char == 'u' || Char == 's') break;
            if((PrevChar == 'v' && Char == 't')) 
            {
                v3 Coords = ParseVertex(Line);
                tex2d UVCoords = V3ToUVs(Coords);
                ArrayPush(TextureCoords, UVCoords, tex2d);
                break;
            }
            if((PrevChar == 'v' && Char == 'n')) break;

            if((PrevChar == 'v' && Char == ' '))
            {
                v3 Vert = ParseVertex(Line);

                ArrayPush(Mesh->Vertices, Vert, v3);
                break;
            } 
            if((PrevChar == 'f' && Char == ' '))
            {
                face_t Face = {};
                parsed_obj Obj = ParseFace(Line);

                memcpy(Face.E, Obj.VertexIndices, sizeof(i32)*3);
                Face.UVs.A_uv = TextureCoords[Obj.TextureIndices[0] - 1];
                Face.UVs.B_uv = TextureCoords[Obj.TextureIndices[1] - 1];
                Face.UVs.C_uv = TextureCoords[Obj.TextureIndices[2] - 1];
                Face.Color = 0xFFFFFFFF;
                ArrayPush(Mesh->Meshes, Face, face_t);
                break;
            }
            PrevChar = Char;
        }
    }
}

static void LoadTexture(mesh_t* Mesh, char* Filename)
{
    upng_t* Image = upng_new_from_file(Filename);
    if(Image != NULL)
    {
        upng_decode(Image);
        if(upng_get_error(Image) == UPNG_EOK)
        {
            Mesh->Texture = Image;
        }
    }
}

void 
LoadMesh(char* ObjectPath, char* TexturePath, v3 Scaling = V3(1, 1, 1), v3 Translating = V3(0, 0, 0), v3 Rotating = V3(0, 0, 0))
{
    mesh_t NewMesh = {};

    //LoadObjFileData(&NewMesh, ObjectPath);
    LoadMeshData(&NewMesh, ObjectPath);
    LoadTexture(&NewMesh, TexturePath);

    NewMesh.Scale = Scaling;
    NewMesh.Translation = Translating;
    NewMesh.Rotation = Rotating;

    Meshes[MeshesCount++] = NewMesh;
}


