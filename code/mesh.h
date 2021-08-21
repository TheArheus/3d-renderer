#if !defined(MESH_H)
#define MESH_H

#define N_CUBE_VERTICES (8)
extern v3 CubeVertices[N_CUBE_VERTICES];

#define N_CUBE_MESHES (6*2)
extern face_t CubeMeshes[N_CUBE_MESHES];

typedef struct
{
    v3* Vertices;
    face_t* Meshes;

    v3 Rotation;
} mesh_t;

extern mesh_t Mesh;

void LoadCubeMeshData(void);
void LoadObjFileData(char);

#endif
