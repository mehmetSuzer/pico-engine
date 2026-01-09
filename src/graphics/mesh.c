
#include "mesh.h"

const pgl_vertex_t cube_vertices[] = {
    // Front Face
    {{Q_M_HALF, Q_M_HALF,   Q_HALF}, {Q_ZERO, Q_ZERO}}, 
    {{  Q_HALF, Q_M_HALF,   Q_HALF}, { Q_ONE, Q_ZERO}},         
    {{  Q_HALF,   Q_HALF,   Q_HALF}, { Q_ONE,  Q_ONE}},       
    {{Q_M_HALF,   Q_HALF,   Q_HALF}, {Q_ZERO,  Q_ONE}},

    // Back Face
    {{  Q_HALF, Q_M_HALF, Q_M_HALF}, {Q_ZERO, Q_ZERO}}, 
    {{Q_M_HALF, Q_M_HALF, Q_M_HALF}, { Q_ONE, Q_ZERO}},     
    {{Q_M_HALF,   Q_HALF, Q_M_HALF}, { Q_ONE,  Q_ONE}},   
    {{  Q_HALF,   Q_HALF, Q_M_HALF}, {Q_ZERO,  Q_ONE}},

    // Right Face
    {{  Q_HALF, Q_M_HALF,   Q_HALF}, {Q_ZERO, Q_ZERO}},   
    {{  Q_HALF, Q_M_HALF, Q_M_HALF}, { Q_ONE, Q_ZERO}},        
    {{  Q_HALF,   Q_HALF, Q_M_HALF}, { Q_ONE,  Q_ONE}},     
    {{  Q_HALF,   Q_HALF,   Q_HALF}, {Q_ZERO,  Q_ONE}},

    // Left Face
    {{Q_M_HALF, Q_M_HALF, Q_M_HALF}, {Q_ZERO, Q_ZERO}},
    {{Q_M_HALF, Q_M_HALF,   Q_HALF}, { Q_ONE, Q_ZERO}},     
    {{Q_M_HALF,   Q_HALF,   Q_HALF}, { Q_ONE,  Q_ONE}},     
    {{Q_M_HALF,   Q_HALF, Q_M_HALF}, {Q_ZERO,  Q_ONE}},

    // Up Face
    {{Q_M_HALF,   Q_HALF,   Q_HALF}, {Q_ZERO, Q_ZERO}},   
    {{  Q_HALF,   Q_HALF,   Q_HALF}, { Q_ONE, Q_ZERO}},       
    {{  Q_HALF,   Q_HALF, Q_M_HALF}, { Q_ONE,  Q_ONE}},     
    {{Q_M_HALF,   Q_HALF, Q_M_HALF}, {Q_ZERO,  Q_ONE}},

    // Down Face
    {{Q_M_HALF, Q_M_HALF, Q_M_HALF}, {Q_ZERO, Q_ZERO}},
    {{  Q_HALF, Q_M_HALF, Q_M_HALF}, { Q_ONE, Q_ZERO}},         
    {{  Q_HALF, Q_M_HALF,   Q_HALF}, { Q_ONE,  Q_ONE}},     
    {{Q_M_HALF, Q_M_HALF,   Q_HALF}, {Q_ZERO,  Q_ONE}},
};

const uint16_t cube_indices[] = {
     0,  1,  2,
     0,  2,  3,
     
     4,  5,  6,  
     4,  6,  7,
     
     8,  9, 10,  
     8, 10, 11,
    
    12, 13, 14, 
    12, 14, 15,
    
    16, 17, 18, 
    16, 18, 19,
    
    20, 21, 22, 
    20, 22, 23,
};

const mesh_t cube_mesh = {
    .vertices = cube_vertices,
    .indices = cube_indices,
    .vertex_count = COUNT_OF(cube_vertices),
    .index_count = COUNT_OF(cube_indices),
};


