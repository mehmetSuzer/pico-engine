
#ifndef PICO_ENGINE_MESH_MESH_H
#define PICO_ENGINE_MESH_MESH_H

#include "pgl/pgl.h"

typedef struct
{
    const pgl_vertex_t* vertices;
    const uint16_t* indices;
    uint16_t vertex_count;
    uint16_t index_count;
} mesh_t;

extern const mesh_t cube_mesh;

#endif // PICO_ENGINE_MESH_MESH_H

