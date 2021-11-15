#if !defined(MESH_H)
#define MESH_H

typedef struct
{
    v3* Vertices;
    face_t* Meshes;
    upng_t* Texture;
    v3 Translation;
    v3 Rotation;
    v3 Scale;
} mesh_t;

extern mesh_t Meshes[1 << 4];
extern i32 MeshesCount;

void LoadMeshData(mesh_t*, char*);
static void LoadTexture(mesh_t* Mesh, char* Filename);
void LoadMesh(char* Object, char* Texture, v3 Scaling, v3 Rotating, v3 Translating);

#endif
