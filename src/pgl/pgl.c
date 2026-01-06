
#include "pgl.h"

// ------------------------------------- TYPES ------------------------------------- //

typedef struct
{
    Q_VEC4 position;
    Q_VEC2 tex_coord;
} pgl_clip_vertex_t;

typedef struct
{
    pgl_clip_vertex_t v0;
    pgl_clip_vertex_t v1;
    pgl_clip_vertex_t v2;
} pgl_clip_triangle_t;

typedef struct
{
    Q_VEC2 tex_coord;
    Q_TYPE inv_depth;
    int32_t x, y;
} pgl_fragment_t;

typedef struct
{
    colour_t colours[SCREEN_HEIGHT][SCREEN_WIDTH];
    depth_t   depths[SCREEN_HEIGHT][SCREEN_WIDTH];
} pgl_framebuffer_t;

typedef struct
{
    pgl_framebuffer_t framebuffer;

    Q_MAT4 model;
    Q_MAT4 view;
    Q_MAT4 projection;
    Q_MAT4 viewport;

    Q_TYPE near;
    Q_TYPE far;

    pgl_texture_t texture;
    
    colour_t clear_colour;
    depth_t  clear_depth;
} pgl_context_t;

static pgl_context_t context = {
    .framebuffer = {
        .colours = {{COLOUR_BLACK}},
        .depths  = {{DEPTH_FURTHEST}},
    },

    .model      = Q_MAT4_ZERO,
    .view       = Q_MAT4_ZERO,
    .projection = Q_MAT4_ZERO,
    .viewport   = Q_MAT4_ZERO,

    .near = Q_ZERO,
    .far  = Q_MAX,

    .texture = {
        .data = NULL,
        .row  = 0u,
        .col  = 0u,
    },

    .clear_colour = COLOUR_BLACK,
    .clear_depth  = DEPTH_FURTHEST,
};

// --------------------------------- TRIANGLE QUEUE --------------------------------- // 

#define PGL_CLIP_QUEUE_CAPACITY 16u

typedef struct
{
    pgl_clip_triangle_t triangles[PGL_CLIP_QUEUE_CAPACITY];
    uint16_t front; // index of the next slot for pop
    uint16_t back;  // index of the next slot for push
} pgl_clip_queue_t;


static inline void pgl_clip_queue_init(pgl_clip_queue_t* queue)
{
    queue->front = 0u;
    queue->back  = 0u;
}

static inline bool pgl_clip_queue_is_empty(const pgl_clip_queue_t* queue)
{
    return (queue->front == queue->back);
}

static inline bool pgl_clip_queue_is_full(const pgl_clip_queue_t* queue)
{
    return ((queue->back + 1u) % PGL_CLIP_QUEUE_CAPACITY == queue->front);
}

static inline uint16_t pgl_clip_queue_length(const pgl_clip_queue_t* queue)
{
    return (queue->back >= queue->front) ? queue->back - queue->front
        : (queue->back + PGL_CLIP_QUEUE_CAPACITY) - queue->front;
}

// WARNING: Does nothing if the queue is full
static inline void pgl_clip_queue_push(pgl_clip_queue_t* queue, const pgl_clip_triangle_t* triangle)
{
    if (pgl_clip_queue_is_full(queue)) return;
    queue->triangles[queue->back] = *triangle;
    queue->back = (queue->back + 1u) % PGL_CLIP_QUEUE_CAPACITY;
}

// WARNING: Returns NULL if the queue is empty
static inline pgl_clip_triangle_t* pgl_clip_queue_pop(pgl_clip_queue_t* queue)
{
    if (pgl_clip_queue_is_empty(queue)) return NULL;
    pgl_clip_triangle_t* triangle = queue->triangles + queue->front;
    queue->front = (queue->front + 1u) % PGL_CLIP_QUEUE_CAPACITY;
    return triangle;
}

// ------------------------------------- CLIP ------------------------------------- // 

static int16_t pgl_clip(
    pgl_clip_triangle_t* restrict clip_triangle, 
    pgl_clip_triangle_t* restrict subtriangles)
{
    static const Q_VEC4 clip_plane_vectors[] = {
        {Q_ZERO, Q_ZERO,  Q_ONE,   Q_ONE}, // Near   : Z + W > 0.0
        { Q_ONE, Q_ZERO, Q_ZERO,   Q_ONE}, // Left   : X + W > 0.0
        { Q_ONE, Q_ZERO, Q_ZERO, Q_M_ONE}, // Right  : X - W < 0.0
        {Q_ZERO,  Q_ONE, Q_ZERO,   Q_ONE}, // Bottom : Y + W > 0.0
        {Q_ZERO,  Q_ONE, Q_ZERO, Q_M_ONE}, // Top    : Y - W < 0.0
        {Q_ZERO, Q_ZERO,  Q_ONE, Q_M_ONE}, // Far    : Z - W < 0.0
    };

    int16_t last_index = -1;
    pgl_clip_queue_t queue;
    pgl_clip_queue_init(&queue);
    pgl_clip_queue_push(&queue, clip_triangle);
    
    for (uint16_t i = 0u; i < COUNT_OF(clip_plane_vectors); ++i)
    {
        while (!pgl_clip_queue_is_empty(&queue)) 
        {
            pgl_clip_triangle_t* tri = pgl_clip_queue_pop(&queue);
            Q_TYPE dot0 = q_vec4_dot(tri->v0.position, clip_plane_vectors[i]);
            Q_TYPE dot1 = q_vec4_dot(tri->v1.position, clip_plane_vectors[i]);
            Q_TYPE dot2 = q_vec4_dot(tri->v2.position, clip_plane_vectors[i]);

            const bool v0_inside = q_gt(q_mul(clip_plane_vectors[i].w, dot0), Q_ZERO);
            const bool v1_inside = q_gt(q_mul(clip_plane_vectors[i].w, dot1), Q_ZERO);
            const bool v2_inside = q_gt(q_mul(clip_plane_vectors[i].w, dot2), Q_ZERO);
            const int16_t num_inside = v0_inside + v1_inside + v2_inside;

            if (num_inside == 3)
            {
                subtriangles[++last_index] = *tri;
            }
            else if (num_inside == 2) 
            {
                // Ensure that v0 is the vertex that is not inside
                if (!v1_inside)
                {
                    SWAP(&tri->v0, &tri->v1);
                    SWAP(&dot0, &dot1);
                }
                else if (!v2_inside)
                {
                    SWAP(&tri->v0, &tri->v2);
                    SWAP(&dot0, &dot2);
                }

                const Q_TYPE alpha10 = q_div(dot0, q_sub(dot0, dot1));
                const Q_TYPE alpha20 = q_div(dot0, q_sub(dot0, dot2));

                const pgl_clip_vertex_t vertex10 = {
                    q_vec4_interp(tri->v1.position,  tri->v0.position,  alpha10),
                    q_vec2_interp(tri->v1.tex_coord, tri->v0.tex_coord, alpha10),
                };
                const pgl_clip_vertex_t vertex20 = {
                    q_vec4_interp(tri->v2.position,  tri->v0.position,  alpha20),
                    q_vec2_interp(tri->v2.tex_coord, tri->v0.tex_coord, alpha20),
                };

                // Preserve the winding order
                if (v0_inside)
                {
                    subtriangles[++last_index] = (pgl_clip_triangle_t){vertex10, tri->v1, tri->v2};
                    subtriangles[++last_index] = (pgl_clip_triangle_t){vertex10, tri->v2, vertex20};
                }
                else
                {
                    subtriangles[++last_index] = (pgl_clip_triangle_t){vertex10, vertex20, tri->v2};
                    subtriangles[++last_index] = (pgl_clip_triangle_t){vertex10, tri->v2, tri->v1};
                }
            }
            else if (num_inside == 1)
            {
                // Ensure that v0 is the vertex that is inside
                if (v1_inside)
                {
                    SWAP(&tri->v0, &tri->v1);
                    SWAP(&dot0, &dot1);
                }
                else if (v2_inside)
                {
                    SWAP(&tri->v0, &tri->v2);
                    SWAP(&dot0, &dot2);
                }

                const Q_TYPE alpha10 = q_div(dot0, q_sub(dot0, dot1));
                const Q_TYPE alpha20 = q_div(dot0, q_sub(dot0, dot2));

                const pgl_clip_vertex_t vertex10 = {
                    q_vec4_interp(tri->v1.position,  tri->v0.position,  alpha10),
                    q_vec2_interp(tri->v1.tex_coord, tri->v0.tex_coord, alpha10),
                };
                const pgl_clip_vertex_t vertex20 = {
                    q_vec4_interp(tri->v2.position,  tri->v0.position,  alpha20),
                    q_vec2_interp(tri->v2.tex_coord, tri->v0.tex_coord, alpha20),
                };

                // Preserve the winding order
                if (v0_inside)
                    subtriangles[++last_index] = (pgl_clip_triangle_t){tri->v0, vertex10, vertex20};
                else
                    subtriangles[++last_index] = (pgl_clip_triangle_t){tri->v0, vertex20, vertex10}; 
            }
        }

        if (i < COUNT_OF(clip_plane_vectors) - 1)
        {
            while (last_index >= 0)
            {
                pgl_clip_queue_push(&queue, subtriangles + last_index);
                last_index--;
            }
        }
    }
    return last_index;
}

// ------------------------------------- TESTS ------------------------------------- //

static inline bool pgl_depth_test_passed(int32_t x, int32_t y, depth_t depth)
{
    // Depth Test -> LESS
    const depth_t depth_in_buffer = context.framebuffer.depths[y][x];
    return (depth < depth_in_buffer);
}

static inline bool pgl_face_is_culled(Q_VEC2 v0_xy_ndc, Q_VEC2 v1_xy_ndc, Q_VEC2 v2_xy_ndc)
{
    // Front Face -> CCW and Cull Face -> BACK
    const Q_VEC2 edge01 = q_vec2_sub(v1_xy_ndc, v0_xy_ndc);
    const Q_VEC2 edge02 = q_vec2_sub(v2_xy_ndc, v0_xy_ndc);
    const Q_TYPE area   = q_vec2_cross(edge01, edge02);
    return (area < Q_ZERO);
}

// ------------------------------------- TEXTURE ------------------------------------- //

static inline colour_t pgl_sample_texture(Q_VEC2 tex_coord)
{
    if (context.texture.data == NULL) return COLOUR_BLACK;
    // TODO: implement texture sampling
    const colour_t place_holder = COLOUR_RED;
    return place_holder;
}

// ------------------------------------- SHADERS ------------------------------------- //

static pgl_clip_vertex_t pgl_vertex_shader(pgl_vertex_t vertex)
{
    const Q_VEC4 point = q_homogeneous_point(vertex.position);
    const Q_VEC4 pos_out = q_mat4_mul_vec4(context.projection, 
        q_mat4_mul_vec4(context.view, q_mat4_mul_vec4(context.model, point)));

    const pgl_clip_vertex_t clip_vertex = {
        .position = pos_out,
        .tex_coord = vertex.tex_coord,
    };
    return clip_vertex;
}

static colour_t pgl_fragment_shader(Q_VEC2 tex_coord)
{
    colour_t colour = pgl_sample_texture(tex_coord);
    // TODO: implement a shading technique
    return colour;
}

// ------------------------------------- RASTERISER ------------------------------------- //

static void pgl_draw_filled_triangle(pgl_fragment_t f0, pgl_fragment_t f1, pgl_fragment_t f2)
{
    // Sort fragments with respect to their y coordinates
    if (f0.y > f1.y) SWAP(&f0, &f1);
    if (f1.y > f2.y) SWAP(&f1, &f2);
    if (f0.y > f1.y) SWAP(&f0, &f1);

    int32_t dx1 = f1.x - f0.x;
    int32_t dy1 = f1.y - f0.y;
    int32_t dx2 = f2.x - f0.x;
    int32_t dy2 = f2.y - f0.y;

    Q_TYPE du1 = q_sub(f1.tex_coord.u, f0.tex_coord.u);
    Q_TYPE dv1 = q_sub(f1.tex_coord.v, f0.tex_coord.v);
    Q_TYPE du2 = q_sub(f2.tex_coord.u, f0.tex_coord.u);
    Q_TYPE dv2 = q_sub(f2.tex_coord.v, f0.tex_coord.v);
    
    Q_TYPE dw1 = q_sub(f1.inv_depth, f0.inv_depth);
    Q_TYPE dw2 = q_sub(f2.inv_depth, f0.inv_depth);

    Q_TYPE inv_abs_dy1 = q_div(Q_ONE, Q_FROM_INT(ABS(dy1)));
    Q_TYPE inv_abs_dy2 = q_div(Q_ONE, Q_FROM_INT(ABS(dy2)));
    
    Q_TYPE sx1 = (dy1 != 0) ? q_mul(Q_FROM_INT(dx1), inv_abs_dy1) : Q_ZERO;
    Q_TYPE su1 = (dy1 != 0) ? q_mul(du1,             inv_abs_dy1) : Q_ZERO;
    Q_TYPE sv1 = (dy1 != 0) ? q_mul(dv1,             inv_abs_dy1) : Q_ZERO;
    Q_TYPE sw1 = (dy1 != 0) ? q_mul(dw1,             inv_abs_dy1) : Q_ZERO;

    Q_TYPE sx2 = (dy2 != 0) ? q_mul(Q_FROM_INT(dx2), inv_abs_dy2) : Q_ZERO;
    Q_TYPE su2 = (dy2 != 0) ? q_mul(du2,             inv_abs_dy2) : Q_ZERO;
    Q_TYPE sv2 = (dy2 != 0) ? q_mul(dv2,             inv_abs_dy2) : Q_ZERO;
    Q_TYPE sw2 = (dy2 != 0) ? q_mul(dw2,             inv_abs_dy2) : Q_ZERO;

	if (dy1 != 0) 
    {
		for (int32_t y = f0.y; y <= f1.y; ++y) 
        {
            const Q_TYPE y0_diff = Q_FROM_INT(y - f0.y);

            int32_t start_x = f0.x + Q_TO_INT(q_mul(y0_diff, sx1));
            Q_TYPE  start_u = q_add(f0.tex_coord.u, q_mul(y0_diff, su1));
            Q_TYPE  start_v = q_add(f0.tex_coord.v, q_mul(y0_diff, sv1));
            Q_TYPE  start_w = q_add(f0.inv_depth, q_mul(y0_diff, sw1));

            int32_t end_x = f0.x + Q_TO_INT(q_mul(y0_diff, sx2));
            Q_TYPE  end_u = q_add(f0.tex_coord.u, q_mul(y0_diff, su2));
            Q_TYPE  end_v = q_add(f0.tex_coord.v, q_mul(y0_diff, sv2));
            Q_TYPE  end_w = q_add(f0.inv_depth, q_mul(y0_diff, sw2));

            if (start_x > end_x) 
            {
				SWAP(&start_x, &end_x);
				SWAP(&start_u, &end_u);
                SWAP(&start_v, &end_v);
                SWAP(&start_w, &end_w);
			}

            Q_TYPE a = Q_ZERO;
			const Q_TYPE sa = q_div(Q_ONE, Q_FROM_INT(end_x - start_x));

			for (int32_t x = start_x; x < end_x; ++x) 
            {
                const Q_TYPE inv_w = q_div(Q_ONE, q_interp(end_w, start_w, a));
                const depth_t depth = depth_map(inv_w, context.near, context.far);

				if (pgl_depth_test_passed(x, y, depth)) 
                {
                    const Q_VEC2 tex_coord = (Q_VEC2){{
                        q_mul(q_interp(end_u, start_u, a), inv_w),
                        q_mul(q_interp(end_v, start_v, a), inv_w),
                    }};
                    const colour_t colour = pgl_fragment_shader(tex_coord);
                    context.framebuffer.colours[y][x] = colour;
                    context.framebuffer.depths [y][x] = depth;
				}
				a += sa;
			}
		}
	}

    dx1 = f2.x - f1.x;
    dy1 = f2.y - f1.y;

    du1 = q_sub(f2.tex_coord.u, f1.tex_coord.u);
    dv1 = q_sub(f2.tex_coord.v, f1.tex_coord.v);
    
    dw1 = q_sub(f2.inv_depth, f1.inv_depth);

    inv_abs_dy1 = q_div(Q_ONE, Q_FROM_INT(ABS(dy1)));
    
    sx1 = (dy1 != 0) ? q_mul(Q_FROM_INT(dx1), inv_abs_dy1) : Q_ZERO;
    su1 = (dy1 != 0) ? q_mul(du1,             inv_abs_dy1) : Q_ZERO;
    sv1 = (dy1 != 0) ? q_mul(dv1,             inv_abs_dy1) : Q_ZERO;
    sw1 = (dy1 != 0) ? q_mul(dw1,             inv_abs_dy1) : Q_ZERO;

	if (dy1 != 0) 
    {
		for (int32_t y = f1.y; y <= f2.y; ++y) 
        {
            const Q_TYPE y0_diff = Q_FROM_INT(y - f0.y);
            const Q_TYPE y1_diff = Q_FROM_INT(y - f1.y);

            int32_t start_x = f1.x + Q_TO_INT(q_mul(y1_diff, sx1));
            Q_TYPE  start_u = q_add(f1.tex_coord.u, q_mul(y1_diff, su1));
            Q_TYPE  start_v = q_add(f1.tex_coord.v, q_mul(y1_diff, sv1));
            Q_TYPE  start_w = q_add(f1.inv_depth, q_mul(y1_diff, sw1));

            int32_t end_x = f0.x + Q_TO_INT(q_mul(y0_diff, sx2));
            Q_TYPE  end_u = q_add(f0.tex_coord.u, q_mul(y0_diff, su2));
            Q_TYPE  end_v = q_add(f0.tex_coord.v, q_mul(y0_diff, sv2));
            Q_TYPE  end_w = q_add(f0.inv_depth, q_mul(y0_diff, sw2));

            if (start_x > end_x) 
            {
				SWAP(&start_x, &end_x);
				SWAP(&start_u, &end_u);
                SWAP(&start_v, &end_v);
                SWAP(&start_w, &end_w);
			}

            Q_TYPE a = Q_ZERO;
			const Q_TYPE sa = q_div(Q_ONE, Q_FROM_INT(end_x - start_x));

			for (int32_t x = start_x; x < end_x; ++x) 
            {
                const Q_TYPE inv_w = q_div(Q_ONE, q_interp(end_w, start_w, a));
                const depth_t depth = depth_map(inv_w, context.near, context.far);

				if (pgl_depth_test_passed(x, y, depth)) 
                {
                    const Q_VEC2 tex_coord = (Q_VEC2){{
                        q_mul(q_interp(end_u, start_u, a), inv_w),
                        q_mul(q_interp(end_v, start_v, a), inv_w),
                    }};
                    const colour_t colour = pgl_fragment_shader(tex_coord);
                    context.framebuffer.colours[y][x] = colour;
                    context.framebuffer.depths [y][x] = depth;
				}
				a += sa;
			}
		}
	}
}

// ------------------------------------- CONTEXT ------------------------------------- //

void pgl_model(Q_VEC3 position, Q_QUAT rotation, Q_VEC3 scale)
{
    context.model = Q_MAT4_IDENTITY;            // M = I
    q_translate_3d(&context.model, position);   // M = I * T
    q_rotate_3d_quat(&context.model, rotation); // M = I * T * R
    q_scale_3d(&context.model, scale);          // M = I * T * R * S
}

void pgl_view(Q_VEC3 eye, Q_VEC3 backward, Q_VEC3 up)
{
    context.view = q_view(eye, backward, up);
}

#define ASPECT_RATIO (Q_FROM_FLOAT((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT))

void pgl_projection(Q_TYPE fovw, Q_TYPE near, Q_TYPE far)
{
    context.projection = q_perspective(fovw, ASPECT_RATIO, near, far);
    context.near = near;
    context.far  = far;
}

void pgl_viewport(int32_t x, int32_t y, uint32_t width, uint32_t height)
{
    context.viewport = q_viewport(x, y, width, height);
}

void pgl_clear_colour(colour_t colour)
{
    context.clear_colour = colour;
}

void pgl_clear_depth(depth_t depth)
{
    context.clear_depth = depth;
}

void pgl_clear(pgl_framebuffer_bit_t bits)
{
    if (bits & PGL_COLOUR_BUFFER_BIT)
    {
        const colour_t colour = context.clear_colour;
    #if defined(RGB332)
        const uint32_t value = (colour << 24) | (colour << 16) | (colour << 8) | colour;
        const uint32_t count = (SCREEN_HEIGHT * SCREEN_WIDTH) / 4;
    #elif defined(RGB565)
        const uint32_t value = (colour << 16) | colour;
        const uint32_t count = (SCREEN_HEIGHT * SCREEN_WIDTH) / 2;
    #endif
        uint32_t* buffer = (uint32_t*)context.framebuffer.colours;

        for (uint32_t i = 0; i < count; ++i)
            buffer[i] = value;
    }

    if (bits & PGL_DEPTH_BUFFER_BIT)
    {
        const depth_t depth = context.clear_depth;
    #if defined(DEPTH_8BIT)
        const uint32_t value = (depth << 24) | (depth << 16) | (depth << 8) | depth;
        const uint32_t count = (SCREEN_HEIGHT * SCREEN_WIDTH) / 4;
    #elif defined(DEPTH_16BIT)
        const uint32_t value = (depth << 16) | depth;
        const uint32_t count = (SCREEN_HEIGHT * SCREEN_WIDTH) / 2;
    #endif
        uint32_t* buffer = (uint32_t*)context.framebuffer.depths;

        for (uint32_t i = 0; i < count; ++i)
            buffer[i] = value;
    }
}

void pgl_bind_texture(const colour_t* data, uint16_t row, uint16_t col)
{
    context.texture.data = data;
    context.texture.row  = row;
    context.texture.col  = col;
}

void pgl_draw(const pgl_vertex_t* vertices, const uint16_t* indices, uint16_t num_indices)
{
    pgl_clip_triangle_t subtriangles[PGL_CLIP_QUEUE_CAPACITY];

    for (uint16_t i = 0; i < num_indices; i+=3)
    {
        pgl_clip_vertex_t v0 = pgl_vertex_shader(vertices[indices[i + 0]]);
        pgl_clip_vertex_t v1 = pgl_vertex_shader(vertices[indices[i + 1]]);
        pgl_clip_vertex_t v2 = pgl_vertex_shader(vertices[indices[i + 2]]);

        pgl_clip_triangle_t clip_triangle = {v0, v1, v2};
        int16_t last_index = pgl_clip(&clip_triangle, subtriangles);

        while (last_index >= 0)
        {
            const pgl_clip_triangle_t* subtriangle = subtriangles + last_index;
            last_index--;

            const Q_VEC4 ndc0 = q_homogeneous_point_normalise(subtriangle->v0.position);
            const Q_VEC4 ndc1 = q_homogeneous_point_normalise(subtriangle->v1.position);
            const Q_VEC4 ndc2 = q_homogeneous_point_normalise(subtriangle->v2.position);

            const Q_VEC2 v0_xy_ndc = (Q_VEC2){{ndc0.x, ndc0.y}};
            const Q_VEC2 v1_xy_ndc = (Q_VEC2){{ndc1.x, ndc1.y}};
            const Q_VEC2 v2_xy_ndc = (Q_VEC2){{ndc2.x, ndc2.y}};

            if (pgl_face_is_culled(v0_xy_ndc, v1_xy_ndc, v2_xy_ndc)) continue;

            const Q_VEC4 s0 = q_mat4_mul_vec4(context.viewport, subtriangle->v0.position);
            const Q_VEC4 s1 = q_mat4_mul_vec4(context.viewport, subtriangle->v1.position);
            const Q_VEC4 s2 = q_mat4_mul_vec4(context.viewport, subtriangle->v2.position);

            const Q_TYPE inv_depth0 = q_div(Q_ONE, subtriangle->v0.position.w);
            const Q_TYPE inv_depth1 = q_div(Q_ONE, subtriangle->v1.position.w);
            const Q_TYPE inv_depth2 = q_div(Q_ONE, subtriangle->v2.position.w);

            const pgl_fragment_t f0 = {
                .tex_coord = q_vec2_scale(subtriangle->v0.tex_coord, inv_depth0),
                .inv_depth = inv_depth0,
                .x = Q_TO_INT(s0.x),
                .y = Q_TO_INT(s0.y),
            };

            const pgl_fragment_t f1 = {
                .tex_coord = q_vec2_scale(subtriangle->v1.tex_coord, inv_depth1),
                .inv_depth = inv_depth1,
                .x = Q_TO_INT(s1.x),
                .y = Q_TO_INT(s1.y),
            };

            const pgl_fragment_t f2 = {
                .tex_coord = q_vec2_scale(subtriangle->v2.tex_coord, inv_depth2),
                .inv_depth = inv_depth2,
                .x = Q_TO_INT(s2.x),
                .y = Q_TO_INT(s2.y),
            };

            pgl_draw_filled_triangle(f0, f1, f2);
        }
    }
}

