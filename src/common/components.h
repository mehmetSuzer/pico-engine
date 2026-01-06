
#ifndef PICO_ENGINE_COMMON_COMPONENTS_H
#define PICO_ENGINE_COMMON_COMPONENTS_H

#include "fixed_point.h"

typedef struct
{
    Q_VEC3 position;
    Q_QUAT rotation;
    Q_VEC3 scale;
} transform_component_t;

typedef struct
{
    Q_VEC3 linear_velocity;
    Q_VEC3 angular_velocity;
    Q_VEC3 total_force;
    Q_VEC3 total_torque;
    Q_MAT3 inertia_tensor;
    Q_TYPE mass;
} rigid_body_component_t;

typedef struct
{
    Q_TYPE fovw;
    Q_TYPE near;
    Q_TYPE far;
} camera_component_t;

typedef struct
{
    uint16_t mesh;
    uint16_t shader_program;
} renderer_component_t;

typedef struct
{
    Q_VEC3 colour;
    Q_TYPE intensity;
} directional_light_component_t;

typedef struct
{
    Q_VEC3 colour;
    Q_TYPE quadratic;
    Q_TYPE linear;
} point_light_component_t;

typedef struct
{
    Q_VEC3 colour;
    Q_TYPE quadratic;
    Q_TYPE linear;
    Q_TYPE cos_inner_angle;
    Q_TYPE cos_outer_angle;
} spot_light_component_t;

#endif // PICO_ENGINE_COMMON_COMPONENTS_H

