
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
    int32_t x, y;
    Q_VEC2 tex_coord;
    Q_TYPE inv_depth;
} pgl_rast_vertex_t;

typedef struct
{
    const colour_t* texels;
    uint width_bits;
    uint height_bits;
} pgl_texture_t;

typedef struct
{
    framebuffer_t* framebuffer;

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
    .framebuffer = NULL,

    .model      = Q_MAT4_ZERO,
    .view       = Q_MAT4_ZERO,
    .projection = Q_MAT4_ZERO,
    .viewport   = Q_MAT4_ZERO,

    .near = Q_ZERO,
    .far  = Q_MAX,

    .texture = {
        .texels      = NULL,
        .width_bits  = 0u,
        .height_bits = 0u,
    },

    .clear_colour = COLOUR_BLACK,
    .clear_depth  = DEPTH_FURTHEST,
};

// ------------------------------------- CLIP BUFFER ------------------------------------- // 

#define PGL_CLIP_BUFFER_CAPACITY 16u

typedef struct
{
    pgl_clip_triangle_t triangles[PGL_CLIP_BUFFER_CAPACITY];
    int32_t size; // index of the first empty slot
} pgl_clip_buffer_t;

static inline void pgl_clip_buffer_push(pgl_clip_buffer_t* buffer, const pgl_clip_triangle_t triangle)
{
    buffer->triangles[buffer->size++] = triangle;
}

static inline pgl_clip_triangle_t* pgl_clip_buffer_pop(pgl_clip_buffer_t* buffer)
{
    buffer->size--;
    return buffer->triangles + buffer->size;
}

// ------------------------------------- CLIP ------------------------------------- // 

static void pgl_clip(pgl_clip_triangle_t* clip_triangle, pgl_clip_buffer_t* buffer_out)
{
    static const Q_VEC4 clip_plane_vectors[] = {
        {Q_ZERO, Q_ZERO,  Q_ONE,   Q_ONE}, // Near   : Z + W > 0.0
        { Q_ONE, Q_ZERO, Q_ZERO,   Q_ONE}, // Left   : X + W > 0.0
        { Q_ONE, Q_ZERO, Q_ZERO, Q_M_ONE}, // Right  : X - W < 0.0
        {Q_ZERO,  Q_ONE, Q_ZERO,   Q_ONE}, // Bottom : Y + W > 0.0
        {Q_ZERO,  Q_ONE, Q_ZERO, Q_M_ONE}, // Top    : Y - W < 0.0
        {Q_ZERO, Q_ZERO,  Q_ONE, Q_M_ONE}, // Far    : Z - W < 0.0
    };

    pgl_clip_buffer_t buffer;

    buffer.size = 0;
    buffer_out->size = 0;

    // The order matters. After 6 iterations, all subtriangles will be listed in the buffer_out.
    pgl_clip_buffer_t* pop_buffer = buffer_out;
    pgl_clip_buffer_t* push_buffer = &buffer;
    pgl_clip_buffer_push(pop_buffer, *clip_triangle);
    
    for (uint16_t i = 0u; i < COUNT_OF(clip_plane_vectors); ++i)
    {
        push_buffer->size = 0;
        while (pop_buffer->size > 0) 
        {
            pgl_clip_triangle_t* tri = pgl_clip_buffer_pop(pop_buffer);
            Q_TYPE dot0 = q_vec4_dot(tri->v0.position, clip_plane_vectors[i]);
            Q_TYPE dot1 = q_vec4_dot(tri->v1.position, clip_plane_vectors[i]);
            Q_TYPE dot2 = q_vec4_dot(tri->v2.position, clip_plane_vectors[i]);

            const bool v0_inside = q_gt(q_mul(clip_plane_vectors[i].w, dot0), Q_ZERO);
            const bool v1_inside = q_gt(q_mul(clip_plane_vectors[i].w, dot1), Q_ZERO);
            const bool v2_inside = q_gt(q_mul(clip_plane_vectors[i].w, dot2), Q_ZERO);
            const int16_t num_inside = v0_inside + v1_inside + v2_inside;

            if (num_inside == 3)
            {
                pgl_clip_buffer_push(push_buffer, *tri);
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
                if (!v0_inside)
                {
                    pgl_clip_buffer_push(push_buffer, (pgl_clip_triangle_t){vertex10, tri->v1, tri->v2});
                    pgl_clip_buffer_push(push_buffer, (pgl_clip_triangle_t){vertex10, tri->v2, vertex20});
                }
                else
                {
                    pgl_clip_buffer_push(push_buffer, (pgl_clip_triangle_t){vertex10, tri->v2, tri->v1});
                    pgl_clip_buffer_push(push_buffer, (pgl_clip_triangle_t){vertex10, vertex20, tri->v2});
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
                    pgl_clip_buffer_push(push_buffer, (pgl_clip_triangle_t){tri->v0, vertex10, vertex20});
                else
                    pgl_clip_buffer_push(push_buffer, (pgl_clip_triangle_t){tri->v0, vertex20, vertex10});
            }
        }
        SWAP(&pop_buffer, &push_buffer);
    }
}

// ------------------------------------- TESTS ------------------------------------- //

static inline depth_t pgl_depth_map(Q_TYPE depth)
{
    const Q_TYPE ratio = q_div(q_sub(depth, context.near), q_sub(context.far, context.near));
    const depth_t depth_bit = Q_TO_INT(q_add(q_mul_int(ratio, DEPTH_RANGE), Q_FROM_INT(DEPTH_NEAREST)));
    return depth_bit;
}

static inline bool pgl_depth_test_passed(int32_t x, int32_t y, depth_t depth)
{
    // Depth Test -> LESS
    const depth_t depth_in_buffer = context.framebuffer->depths[y][x];
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
// REQUIREMENT: du and dv must be non-negative
void pgl_multisample_texture(colour_t *output, Q_TYPE u, Q_TYPE v, Q_TYPE du, Q_TYPE dv, uint32_t count) 
{
    interp0->accum[0] = q_mul_pow_2(u, context.texture.width_bits); 
    interp0->base[0] = q_mul_pow_2(du, context.texture.width_bits);
    interp0->accum[1] = q_mul_pow_2(v, context.texture.height_bits);
    interp0->base[1] = q_mul_pow_2(dv, context.texture.height_bits);

    for (uint32_t i = 0; i < count; ++i) 
    {
        // equivalent to
        // uint32_t x = (accum0 >> Q_FRAC_BITS) & ((1 << width_bits)  - 1);
        // uint32_t y = (accum1 >> Q_FRAC_BITS) & ((1 << height_bits) - 1);
        // const colour_t* *address = texture + ((x + (y << width_bits)) << bpp_shift);
        // output[i] = *address;
        // accum0 = du + accum0;
        // accum1 = dv + accum1;

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

static colour_t pgl_fragment_shader(Q_VEC2 tex_coord)
{
    colour_t colour;
    pgl_multisample_texture(&colour, tex_coord.u, tex_coord.v, Q_ZERO, Q_ZERO, 1);
    return colour;
}

// ------------------------------------- RASTERISER ------------------------------------- //

static void pgl_draw_filled_triangle(pgl_rast_vertex_t v0, pgl_rast_vertex_t v1, pgl_rast_vertex_t v2)
{
    // Sort vertices with respect to their y coordinates
    // v0.y <= v1.y <= v2.y
    if (v0.y > v1.y) SWAP(&v0, &v1);
    if (v1.y > v2.y) SWAP(&v1, &v2);
    if (v0.y > v1.y) SWAP(&v0, &v1);

    int32_t dx1 = v1.x - v0.x;
    int32_t dy1 = v1.y - v0.y;
    int32_t dx2 = v2.x - v0.x;
    int32_t dy2 = v2.y - v0.y;

    Q_TYPE du1 = q_sub(v1.tex_coord.u, v0.tex_coord.u);
    Q_TYPE dv1 = q_sub(v1.tex_coord.v, v0.tex_coord.v);
    Q_TYPE du2 = q_sub(v2.tex_coord.u, v0.tex_coord.u);
    Q_TYPE dv2 = q_sub(v2.tex_coord.v, v0.tex_coord.v);
    
    Q_TYPE dw1 = q_sub(v1.inv_depth, v0.inv_depth);
    Q_TYPE dw2 = q_sub(v2.inv_depth, v0.inv_depth);

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
		for (int32_t y = v0.y; y <= v1.y; ++y) 
        {
            const Q_TYPE y0_diff = Q_FROM_INT(y - v0.y);

            int32_t start_x = v0.x + Q_TO_INT(q_mul(y0_diff, sx1));
            Q_TYPE  start_u = q_add(v0.tex_coord.u, q_mul(y0_diff, su1));
            Q_TYPE  start_v = q_add(v0.tex_coord.v, q_mul(y0_diff, sv1));
            Q_TYPE  start_w = q_add(v0.inv_depth, q_mul(y0_diff, sw1));

            int32_t end_x = v0.x + Q_TO_INT(q_mul(y0_diff, sx2));
            Q_TYPE  end_u = q_add(v0.tex_coord.u, q_mul(y0_diff, su2));
            Q_TYPE  end_v = q_add(v0.tex_coord.v, q_mul(y0_diff, sv2));
            Q_TYPE  end_w = q_add(v0.inv_depth, q_mul(y0_diff, sw2));

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
                const depth_t depth = pgl_depth_map(inv_w);

				if (pgl_depth_test_passed(x, y, depth)) 
                {
                    const Q_VEC2 tex_coord = (Q_VEC2){{
                        q_mul(q_interp(end_u, start_u, a), inv_w),
                        q_mul(q_interp(end_v, start_v, a), inv_w),
                    }};
                    const colour_t colour = pgl_fragment_shader(tex_coord);
                    context.framebuffer->colours[y][x] = colour;
                    context.framebuffer->depths [y][x] = depth;
				}
				a += sa;
			}
		}
	}

    dx1 = v2.x - v1.x;
    dy1 = v2.y - v1.y;

    du1 = q_sub(v2.tex_coord.u, v1.tex_coord.u);
    dv1 = q_sub(v2.tex_coord.v, v1.tex_coord.v);
    
    dw1 = q_sub(v2.inv_depth, v1.inv_depth);

    inv_abs_dy1 = q_div(Q_ONE, Q_FROM_INT(ABS(dy1)));
    
    sx1 = (dy1 != 0) ? q_mul(Q_FROM_INT(dx1), inv_abs_dy1) : Q_ZERO;
    su1 = (dy1 != 0) ? q_mul(du1,             inv_abs_dy1) : Q_ZERO;
    sv1 = (dy1 != 0) ? q_mul(dv1,             inv_abs_dy1) : Q_ZERO;
    sw1 = (dy1 != 0) ? q_mul(dw1,             inv_abs_dy1) : Q_ZERO;

	if (dy1 != 0) 
    {
		for (int32_t y = v1.y; y <= v2.y; ++y) 
        {
            const Q_TYPE y0_diff = Q_FROM_INT(y - v0.y);
            const Q_TYPE y1_diff = Q_FROM_INT(y - v1.y);

            int32_t start_x = v1.x + Q_TO_INT(q_mul(y1_diff, sx1));
            Q_TYPE  start_u = q_add(v1.tex_coord.u, q_mul(y1_diff, su1));
            Q_TYPE  start_v = q_add(v1.tex_coord.v, q_mul(y1_diff, sv1));
            Q_TYPE  start_w = q_add(v1.inv_depth, q_mul(y1_diff, sw1));

            int32_t end_x = v0.x + Q_TO_INT(q_mul(y0_diff, sx2));
            Q_TYPE  end_u = q_add(v0.tex_coord.u, q_mul(y0_diff, su2));
            Q_TYPE  end_v = q_add(v0.tex_coord.v, q_mul(y0_diff, sv2));
            Q_TYPE  end_w = q_add(v0.inv_depth, q_mul(y0_diff, sw2));

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
                const depth_t depth = pgl_depth_map(inv_w);

				if (pgl_depth_test_passed(x, y, depth)) 
                {
                    const Q_VEC2 tex_coord = (Q_VEC2){{
                        q_mul(q_interp(end_u, start_u, a), inv_w),
                        q_mul(q_interp(end_v, start_v, a), inv_w),
                    }};
                    const colour_t colour = pgl_fragment_shader(tex_coord);
                    context.framebuffer->colours[y][x] = colour;
                    context.framebuffer->depths [y][x] = depth;
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
        uint32_t* buffer = (uint32_t*)context.framebuffer->colours;

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
        uint32_t* buffer = (uint32_t*)context.framebuffer->depths;

        for (uint32_t i = 0; i < count; ++i)
            buffer[i] = value;
    }
}

bool pgl_request_frame()
{
    context.framebuffer = swapchain_acquire_draw_image();
    return (context.framebuffer != NULL);
}

void pgl_bind_texture(const colour_t* texels, uint width_bits, uint height_bits)
{
    context.texture.texels = texels;
    context.texture.width_bits = width_bits;
    context.texture.height_bits = height_bits;

#if defined(RGB332)
    const uint bpp_shift = 0; // log2(1 byte)
#elif defined(RGB565)
    const uint bpp_shift = 1; // log2(2 bytes)
#endif

    interp_config cfg0 = interp_default_config();
    interp_config_set_add_raw(&cfg0, true);
    interp_config_set_shift(&cfg0, Q_FRAC_BITS - bpp_shift);
    interp_config_set_mask(&cfg0, bpp_shift, width_bits + bpp_shift - 1);
    interp_set_config(interp0, 0, &cfg0);

    interp_config cfg1 = interp_default_config();
    interp_config_set_add_raw(&cfg1, true);
    interp_config_set_shift(&cfg1, Q_FRAC_BITS - width_bits - bpp_shift);
    interp_config_set_mask(&cfg1, width_bits + bpp_shift, width_bits + height_bits + bpp_shift - 1);
    interp_set_config(interp0, 1, &cfg1);

    interp0->base[2] = (uintptr_t)context.texture.texels;
}

void pgl_draw(const pgl_vertex_t* vertices, const uint16_t* indices, uint16_t index_count)
{
    pgl_clip_buffer_t clip_buffer;
    for (uint16_t i = 0; i < index_count; i+=3)
    {
        pgl_clip_vertex_t cv0 = pgl_vertex_shader(vertices[indices[i + 0]]);
        pgl_clip_vertex_t cv1 = pgl_vertex_shader(vertices[indices[i + 1]]);
        pgl_clip_vertex_t cv2 = pgl_vertex_shader(vertices[indices[i + 2]]);

        pgl_clip_triangle_t clip_triangle = {cv0, cv1, cv2};
        pgl_clip(&clip_triangle, &clip_buffer);

        while (clip_buffer.size > 0)
        {
            const pgl_clip_triangle_t* subtriangle = pgl_clip_buffer_pop(&clip_buffer);

            const Q_VEC4 ndc0 = q_homogeneous_point_normalise(subtriangle->v0.position);
            const Q_VEC4 ndc1 = q_homogeneous_point_normalise(subtriangle->v1.position);
            const Q_VEC4 ndc2 = q_homogeneous_point_normalise(subtriangle->v2.position);

            const Q_VEC2 v0_xy_ndc = (Q_VEC2){{ndc0.x, ndc0.y}};
            const Q_VEC2 v1_xy_ndc = (Q_VEC2){{ndc1.x, ndc1.y}};
            const Q_VEC2 v2_xy_ndc = (Q_VEC2){{ndc2.x, ndc2.y}};

            if (pgl_face_is_culled(v0_xy_ndc, v1_xy_ndc, v2_xy_ndc)) continue;

            const Q_VEC4 sc0 = q_mat4_mul_vec4(context.viewport, ndc0);
            const Q_VEC4 sc1 = q_mat4_mul_vec4(context.viewport, ndc1);
            const Q_VEC4 sc2 = q_mat4_mul_vec4(context.viewport, ndc2);

            const Q_TYPE inv_depth0 = q_div(Q_ONE, subtriangle->v0.position.w);
            const Q_TYPE inv_depth1 = q_div(Q_ONE, subtriangle->v1.position.w);
            const Q_TYPE inv_depth2 = q_div(Q_ONE, subtriangle->v2.position.w);

            const pgl_rast_vertex_t rv0 = {
                .x = Q_TO_INT(sc0.x),
                .y = Q_TO_INT(sc0.y),
                .tex_coord = q_vec2_scale(subtriangle->v0.tex_coord, inv_depth0),
                .inv_depth = inv_depth0,
            };

            const pgl_rast_vertex_t rv1 = {
                .x = Q_TO_INT(sc1.x),
                .y = Q_TO_INT(sc1.y),
                .tex_coord = q_vec2_scale(subtriangle->v1.tex_coord, inv_depth1),
                .inv_depth = inv_depth1,
            };

            const pgl_rast_vertex_t rv2 = {
                .x = Q_TO_INT(sc2.x),
                .y = Q_TO_INT(sc2.y),
                .tex_coord = q_vec2_scale(subtriangle->v2.tex_coord, inv_depth2),
                .inv_depth = inv_depth2,
            };

            pgl_draw_filled_triangle(rv0, rv1, rv2);
        }
    }
}

