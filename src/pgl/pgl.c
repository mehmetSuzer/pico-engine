
#include <stdio.h>
#include "pgl.h"

// ------------------------------------- TYPES ------------------------------------- //

#define CLIP_POLY_MAX_VERTEX    8
#define CLIP_BUFFER_SIZE        8

typedef struct
{
    Q_VEC4 position;
    Q_VEC2 tex_coord;
} pgl_clip_vertex_t;

typedef struct
{
    pgl_clip_vertex_t verts[3];
} pgl_clip_triangle_t;

typedef struct {
    pgl_clip_vertex_t verts[CLIP_POLY_MAX_VERTEX];
    uint32_t count;
} pgl_clip_poly_t;

typedef struct {
    Q_VEC4 normal;
} pgl_clip_plane_t;

typedef struct
{
    int32_t x, y;
    Q_TYPE  u, v;
    Q_TYPE inv_depth;
} pgl_rast_vertex_t;

typedef struct
{
    depth_t depths[SCREEN_HEIGHT][SCREEN_WIDTH];
    swapchain_image_t* draw_image;

    Q_MAT4 model;
    Q_MAT4 view;
    Q_MAT4 projection;
    Q_MAT4 viewport;

    Q_TYPE near;
    Q_TYPE far;
} pgl_context_t;

static pgl_context_t context = {
    .depths = {{DEPTH_FURTHEST}},
    .draw_image = NULL,

    .model      = Q_MAT4_ZERO,
    .view       = Q_MAT4_ZERO,
    .projection = Q_MAT4_ZERO,
    .viewport   = Q_MAT4_ZERO,

    .near = Q_ZERO,
    .far  = Q_MAX,
};

// ------------------------------------- CLIP ------------------------------------- // 

static void pgl_clip_poly_plane(
    const pgl_clip_poly_t* restrict in,
    const pgl_clip_plane_t* plane,
    pgl_clip_poly_t* restrict out)
{
    out->count = 0;

    uint32_t curr_index = 0;
    uint32_t prev_index = in->count - 1;

    while (curr_index < in->count)
    {
        const pgl_clip_vertex_t* curr = &in->verts[curr_index];
        const pgl_clip_vertex_t* prev = &in->verts[prev_index];

        prev_index = curr_index++;

        const Q_TYPE curr_dot = q_vec4_dot(curr->position, plane->normal);
        const Q_TYPE prev_dot = q_vec4_dot(prev->position, plane->normal);

        const bool curr_inside = q_gt(curr_dot, Q_ZERO);
        const bool prev_inside = q_gt(prev_dot, Q_ZERO);

        if (curr_inside != prev_inside)
        {
            const Q_TYPE t  = q_div(curr_dot, q_sub(curr_dot, prev_dot));
            out->verts[out->count++] = (pgl_clip_vertex_t){
                .position  = q_vec4_interp(prev->position,  curr->position,  t),
                .tex_coord = q_vec2_interp(prev->tex_coord, curr->tex_coord, t),
            };
        }
        
        if (curr_inside)
            out->verts[out->count++] = *curr;
    }
}

static uint32_t pgl_clip(const pgl_clip_triangle_t* restrict clip_triangle, pgl_clip_triangle_t* restrict triangle_out)
{
    static const pgl_clip_plane_t planes[] = {
        {{ Q_ZERO,  Q_ZERO,   Q_ONE, Q_ONE}}, // Near     : +Z + W > 0.0
        {{  Q_ONE,  Q_ZERO,  Q_ZERO, Q_ONE}}, // Left     : +X + W > 0.0
        {{Q_M_ONE,  Q_ZERO,  Q_ZERO, Q_ONE}}, // Right    : -X + W > 0.0
        {{ Q_ZERO,   Q_ONE,  Q_ZERO, Q_ONE}}, // Bottom   : +Y + W > 0.0
        {{ Q_ZERO, Q_M_ONE,  Q_ZERO, Q_ONE}}, // Top      : -Y + W > 0.0
        {{ Q_ZERO,  Q_ZERO, Q_M_ONE, Q_ONE}}, // Far      : -Z + W > 0.0
    };

    pgl_clip_poly_t poly0;
    pgl_clip_poly_t poly1;
    poly0.verts[0] = clip_triangle->verts[0];
    poly0.verts[1] = clip_triangle->verts[1];
    poly0.verts[2] = clip_triangle->verts[2];
    poly0.count = 3;

    pgl_clip_poly_t* poly_in  = &poly0;
    pgl_clip_poly_t* poly_out = &poly1;

    for (uint32_t i = 0; i < COUNT_OF(planes) && poly_in->count > 0; ++i)
    {
        pgl_clip_poly_plane(poly_in, &planes[i], poly_out);
        SWAP(&poly_in, &poly_out);
    }

    uint32_t triangle_count = 0;
    for (uint32_t i = 1; i + 1 < poly_in->count; ++i)
    {
        triangle_out[triangle_count++] = (pgl_clip_triangle_t){
            poly_in->verts[0], 
            poly_in->verts[i], 
            poly_in->verts[i + 1],
        };
    }
    return triangle_count;
}

// ------------------------------------- TESTS ------------------------------------- //

static inline depth_t pgl_depth_map(Q_TYPE q_depth)
{
    const Q_TYPE ratio = q_div(q_sub(q_depth, context.near), q_sub(context.far, context.near));
    const depth_t depth = Q_TO_INT(q_add(q_mul_int(ratio, DEPTH_RANGE), Q_FROM_INT(DEPTH_NEAREST)));
    return depth;
}

static inline bool pgl_depth_test_passed(int32_t x, int32_t y, depth_t depth)
{
    // Depth Test -> LESS
    const depth_t depth_in_buffer = context.depths[y][x];
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

// REQUIREMENT:  u and  v must be non-negative
// REQUIREMENT: su and sv must be non-negative
void pgl_multisample_texture(colour_t *output, Q_TYPE u, Q_TYPE v, Q_TYPE su, Q_TYPE sv, uint32_t count) 
{
    interp0->accum[0] = u;
    interp0->base[0] = su;
    interp0->accum[1] = v;
    interp0->base[1] = sv;

    for (uint32_t i = 0; i < count; ++i) 
    {
        // equivalent to
        // uint32_t x = (accum0 >> (Q_FRAC_BITS - width_bits))  & ((1 << width_bits)  - 1);
        // uint32_t y = (accum1 >> (Q_FRAC_BITS - height_bits)) & ((1 << height_bits) - 1);
        // const colour_t* *address = texture + ((x + (y << width_bits)) << bpp_shift);
        // output[i] = *address;
        // accum0 = su + accum0;
        // accum1 = sv + accum1;

        // popping the result advances to the next iteration
        output[i] = *(const colour_t*)interp0->pop[2];
    }
}

// ------------------------------------- SHADERS ------------------------------------- //

static pgl_clip_vertex_t pgl_vertex_shader(pgl_vertex_t vertex)
{
    const Q_VEC4 point = q_homogeneous_point(vertex.position);
    const Q_VEC4 pos_out = q_mat4_mul_vec4(context.projection, q_mat4_mul_vec4(context.view, q_mat4_mul_vec4(context.model, point)));

    const pgl_clip_vertex_t clip_vertex = {
        .position = pos_out,
        .tex_coord = vertex.tex_coord,
    };
    return clip_vertex;
}

static colour_t pgl_fragment_shader(Q_TYPE u, Q_TYPE v)
{
    colour_t colour;
    pgl_multisample_texture(&colour, u, v, Q_ZERO, Q_ZERO, 1);
    return colour;
}

// ------------------------------------- RASTERISER ------------------------------------- //

static inline void pgl_rasterise_scanline(
    Q_TYPE left_x, Q_TYPE right_x,
    Q_TYPE left_u, Q_TYPE right_u,
    Q_TYPE left_v, Q_TYPE right_v,
    Q_TYPE left_w, Q_TYPE right_w,
    int32_t y)
{
    Q_TYPE u = left_u;
    Q_TYPE v = left_v;
    Q_TYPE w = left_w;

    const Q_TYPE x_diff = q_sub(right_x, left_x);

    const Q_TYPE su = q_ne(x_diff, Q_ZERO) ? q_div(q_sub(right_u, left_u), x_diff) : Q_ZERO;
    const Q_TYPE sv = q_ne(x_diff, Q_ZERO) ? q_div(q_sub(right_v, left_v), x_diff) : Q_ZERO;
    const Q_TYPE sw = q_ne(x_diff, Q_ZERO) ? q_div(q_sub(right_w, left_w), x_diff) : Q_ZERO;

    const int32_t left  = Q_TO_INT(left_x);
    const int32_t right = Q_TO_INT(right_x);

    for (int32_t x = left; x <= right; ++x)
    {
        const Q_TYPE inv_w = q_div(Q_ONE, w);
        const depth_t depth = pgl_depth_map(inv_w);

		if (pgl_depth_test_passed(x, y, depth))
        {
            const colour_t colour = pgl_fragment_shader(
                q_mul(u, inv_w), q_mul(v, inv_w));

            context.draw_image->colours[y][x] = colour;
            context.depths[y][x] = depth;
		}

        u = q_add(u, su);
        v = q_add(v, sv);
        w = q_add(w, sw);
	}
}

static void pgl_rasterise_filled_triangle(pgl_rast_vertex_t vert0, pgl_rast_vertex_t vert1, pgl_rast_vertex_t vert2)
{
    // Sort vertices with respect to their y coordinates
    // vert0.y <= vert1.y <= vert2.y
    if (vert0.y > vert1.y) { SWAP(&vert0, &vert1); }
    if (vert1.y > vert2.y) { SWAP(&vert1, &vert2); }
    if (vert0.y > vert1.y) { SWAP(&vert0, &vert1); }

    const Q_TYPE dx20 = Q_FROM_INT(vert2.x - vert0.x);
    const Q_TYPE dy20 = Q_FROM_INT(vert2.y - vert0.y);
    const Q_TYPE du20 = q_sub(vert2.u, vert0.u);
    const Q_TYPE dv20 = q_sub(vert2.v, vert0.v);
    const Q_TYPE dw20 = q_sub(vert2.inv_depth, vert0.inv_depth);

    const Q_TYPE inv_dy20 = q_div(Q_ONE, dy20);
    const Q_TYPE sx20 = q_mul(dx20, inv_dy20);
    const Q_TYPE su20 = q_mul(du20, inv_dy20);
    const Q_TYPE sv20 = q_mul(dv20, inv_dy20);
    const Q_TYPE sw20 = q_mul(dw20, inv_dy20);

    if (vert1.y != vert0.y)
    {
        const Q_TYPE dx10 = Q_FROM_INT(vert1.x - vert0.x);
        const Q_TYPE dy10 = Q_FROM_INT(vert1.y - vert0.y);
        const Q_TYPE du10 = q_sub(vert1.u, vert0.u);
        const Q_TYPE dv10 = q_sub(vert1.v, vert0.v);
        const Q_TYPE dw10 = q_sub(vert1.inv_depth, vert0.inv_depth);

        const Q_TYPE inv_dy10 = q_div(Q_ONE, dy10);
        const Q_TYPE sx10 = q_mul(dx10, inv_dy10);
        const Q_TYPE su10 = q_mul(du10, inv_dy10);
        const Q_TYPE sv10 = q_mul(dv10, inv_dy10);
        const Q_TYPE sw10 = q_mul(dw10, inv_dy10);

        Q_TYPE left_x = Q_FROM_INT(vert0.x);
        Q_TYPE left_u = vert0.u;
        Q_TYPE left_v = vert0.v;
        Q_TYPE left_w = vert0.inv_depth;

        Q_TYPE right_x = Q_FROM_INT(vert0.x);
        Q_TYPE right_u = vert0.u;
        Q_TYPE right_v = vert0.v;
        Q_TYPE right_w = vert0.inv_depth;
    
        Q_TYPE lx, rx;
        Q_TYPE lu, ru;
        Q_TYPE lv, rv;
        Q_TYPE lw, rw;

        if (q_lt(sx10, sx20))
        {
            lx = sx10; rx = sx20;
            lu = su10; ru = su20;
            lv = sv10; rv = sv20;
            lw = sw10; rw = sw20;
        }
        else
        {
            lx = sx20; rx = sx10;
            lu = su20; ru = su10;
            lv = sv20; rv = sv10;
            lw = sw20; rw = sw10;
        }

    	for (int32_t y = vert0.y; y < vert1.y; ++y)
        {
            pgl_rasterise_scanline(
                left_x, right_x,
                left_u, right_u,
                left_v, right_v,
                left_w, right_w,
                y);

            left_x = q_add(left_x, lx);
            left_u = q_add(left_u, lu);
            left_v = q_add(left_v, lv);
            left_w = q_add(left_w, lw);

            right_x = q_add(right_x, rx);
            right_u = q_add(right_u, ru);
            right_v = q_add(right_v, rv);
            right_w = q_add(right_w, rw);
    	}
    }

    if (vert2.y != vert1.y)
    {
        const Q_TYPE dx21 = Q_FROM_INT(vert2.x - vert1.x);
        const Q_TYPE dy21 = Q_FROM_INT(vert2.y - vert1.y);
        const Q_TYPE du21 = q_sub(vert2.u, vert1.u);
        const Q_TYPE dv21 = q_sub(vert2.v, vert1.v);
        const Q_TYPE dw21 = q_sub(vert2.inv_depth, vert1.inv_depth);

        const Q_TYPE inv_dy21 = q_div(Q_ONE, dy21);
        const Q_TYPE sx21 = q_mul(dx21, inv_dy21);
        const Q_TYPE su21 = q_mul(du21, inv_dy21);
        const Q_TYPE sv21 = q_mul(dv21, inv_dy21);
        const Q_TYPE sw21 = q_mul(dw21, inv_dy21);

        Q_TYPE left_x = Q_FROM_INT(vert2.x);
        Q_TYPE left_u = vert2.u;
        Q_TYPE left_v = vert2.v;
        Q_TYPE left_w = vert2.inv_depth;

        Q_TYPE right_x = Q_FROM_INT(vert2.x);
        Q_TYPE right_u = vert2.u;
        Q_TYPE right_v = vert2.v;
        Q_TYPE right_w = vert2.inv_depth;

        Q_TYPE lx, rx;
        Q_TYPE lu, ru;
        Q_TYPE lv, rv;
        Q_TYPE lw, rw;

        if (q_lt(sx20, sx21))
        {
            lx = sx21; rx = sx20;
            lu = su21; ru = su20;
            lv = sv21; rv = sv20;
            lw = sw21; rw = sw20;
        }
        else
        {
            lx = sx20; rx = sx21;
            lu = su20; ru = su21;
            lv = sv20; rv = sv21;
            lw = sw20; rw = sw21;
        }

    	for (int32_t y = vert2.y; y >= vert1.y; --y)
        {
            pgl_rasterise_scanline(
                left_x, right_x,
                left_u, right_u,
                left_v, right_v,
                left_w, right_w,
                y);

            left_x = q_sub(left_x, lx);
            left_u = q_sub(left_u, lu);
            left_v = q_sub(left_v, lv);
            left_w = q_sub(left_w, lw);

            right_x = q_sub(right_x, rx);
            right_u = q_sub(right_u, ru);
            right_v = q_sub(right_v, rv);
            right_w = q_sub(right_w, rw);
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

void pgl_clear_colours(colour_t colour)
{
#if defined(RGB332)
    const uint32_t value = (colour << 24) | (colour << 16) | (colour << 8) | colour;
    const uint32_t count = (SCREEN_HEIGHT * SCREEN_WIDTH) / 4;
#elif defined(RGB565)
    const uint32_t value = (colour << 16) | colour;
    const uint32_t count = (SCREEN_HEIGHT * SCREEN_WIDTH) / 2;
#endif
    uint32_t* buffer = (uint32_t*)context.draw_image->colours;

    #pragma GCC unroll 16
    for (uint32_t i = 0; i < count; ++i)
        buffer[i] = value;
}

void pgl_clear_depths(depth_t depth)
{
#if defined(DEPTH_8BIT)
    const uint32_t value = (depth << 24) | (depth << 16) | (depth << 8) | depth;
    const uint32_t count = (SCREEN_HEIGHT * SCREEN_WIDTH) / 4;
#elif defined(DEPTH_16BIT)
    const uint32_t value = (depth << 16) | depth;
    const uint32_t count = (SCREEN_HEIGHT * SCREEN_WIDTH) / 2;
#endif
    uint32_t* buffer = (uint32_t*)context.depths;

    #pragma GCC unroll 16
    for (uint32_t i = 0; i < count; ++i)
        buffer[i] = value;
}

bool pgl_request_draw_image()
{
    context.draw_image = swapchain_request_draw_image();
    return (context.draw_image != NULL);
}

void pgl_bind_texture(const colour_t* texels, uint width_bits, uint height_bits)
{
#if defined(RGB332)
    const uint bpp_shift = 0; // log2(1 byte)
#elif defined(RGB565)
    const uint bpp_shift = 1; // log2(2 bytes)
#endif

    interp_config cfg0 = interp_default_config();
    interp_config_set_add_raw(&cfg0, true);
    interp_config_set_shift(&cfg0, Q_FRAC_BITS - width_bits - bpp_shift);
    interp_config_set_mask(&cfg0, bpp_shift, width_bits + bpp_shift - 1);
    interp_set_config(interp0, 0, &cfg0);

    interp_config cfg1 = interp_default_config();
    interp_config_set_add_raw(&cfg1, true);
    interp_config_set_shift(&cfg1, Q_FRAC_BITS - height_bits - width_bits - bpp_shift);
    interp_config_set_mask(&cfg1, width_bits + bpp_shift, width_bits + height_bits + bpp_shift - 1);
    interp_set_config(interp0, 1, &cfg1);

    interp0->base[2] = (uintptr_t)texels;
}

void pgl_draw(const pgl_vertex_t* vertices, const uint16_t* indices, uint16_t index_count)
{
    pgl_clip_triangle_t clip_buffer[CLIP_BUFFER_SIZE];
    for (uint16_t i = 0; i < index_count; i+=3)
    {
        const pgl_clip_triangle_t clip_triangle = {{
            pgl_vertex_shader(vertices[indices[i + 0]]),
            pgl_vertex_shader(vertices[indices[i + 1]]),
            pgl_vertex_shader(vertices[indices[i + 2]]),
        }};
        uint32_t triangle_count = pgl_clip(&clip_triangle, (pgl_clip_triangle_t*)clip_buffer);

        while (triangle_count > 0)
        {
            const pgl_clip_triangle_t* subtriangle = &clip_buffer[--triangle_count];

            const Q_VEC4 ndc0 = q_homogeneous_point_normalise(subtriangle->verts[0].position);
            const Q_VEC4 ndc1 = q_homogeneous_point_normalise(subtriangle->verts[1].position);
            const Q_VEC4 ndc2 = q_homogeneous_point_normalise(subtriangle->verts[2].position);

            if (pgl_face_is_culled((Q_VEC2){{ndc0.x, ndc0.y}}, (Q_VEC2){{ndc1.x, ndc1.y}}, (Q_VEC2){{ndc2.x, ndc2.y}})) 
                continue;

            const Q_VEC4 sc0 = q_mat4_mul_vec4(context.viewport, ndc0);
            const Q_VEC4 sc1 = q_mat4_mul_vec4(context.viewport, ndc1);
            const Q_VEC4 sc2 = q_mat4_mul_vec4(context.viewport, ndc2);

            const Q_TYPE inv_depth0 = q_div(Q_ONE, subtriangle->verts[0].position.w);
            const Q_TYPE inv_depth1 = q_div(Q_ONE, subtriangle->verts[1].position.w);
            const Q_TYPE inv_depth2 = q_div(Q_ONE, subtriangle->verts[2].position.w);

            const pgl_rast_vertex_t rast_vert0 = {
                .x = CLAMP(Q_TO_INT(sc0.x), 0, SCREEN_WIDTH  - 1),
                .y = CLAMP(Q_TO_INT(sc0.y), 0, SCREEN_HEIGHT - 1),
                .u = q_mul(subtriangle->verts[0].tex_coord.u, inv_depth0),
                .v = q_mul(subtriangle->verts[0].tex_coord.v, inv_depth0),
                .inv_depth = inv_depth0,
            };

            const pgl_rast_vertex_t rast_vert1 = {
                .x = CLAMP(Q_TO_INT(sc1.x), 0, SCREEN_WIDTH  - 1),
                .y = CLAMP(Q_TO_INT(sc1.y), 0, SCREEN_HEIGHT - 1),
                .u = q_mul(subtriangle->verts[1].tex_coord.u, inv_depth1),
                .v = q_mul(subtriangle->verts[1].tex_coord.v, inv_depth1),
                .inv_depth = inv_depth1,
            };

            const pgl_rast_vertex_t rast_vert2 = {
                .x = CLAMP(Q_TO_INT(sc2.x), 0, SCREEN_WIDTH  - 1),
                .y = CLAMP(Q_TO_INT(sc2.y), 0, SCREEN_HEIGHT - 1),
                .u = q_mul(subtriangle->verts[2].tex_coord.u, inv_depth2),
                .v = q_mul(subtriangle->verts[2].tex_coord.v, inv_depth2),
                .inv_depth = inv_depth2,
            };

            pgl_rasterise_filled_triangle(rast_vert0, rast_vert1, rast_vert2);
        }
    }
}

