#ifndef _STATIC_MESH_SHADER_H_
#define _STATIC_MESH_SHADER_H_

// note we're only able to use #version 100 since the code needs to be glsl ES compilent.
static constexpr char* STATIC_MESH_VERTEX_SHADER = R"(#version 100

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

attribute vec3 vs_in_position;
attribute vec2 vs_in_uv;
attribute vec3 vs_in_normal;

varying vec3 vs_out_world_position;
varying vec2 vs_out_uv;
varying vec3 vs_out_normal;

void main()
{
    vec4 world_position = u_model * vec4(vs_in_position, 1.0);
    vs_out_world_position = world_position.xyz;
    vs_out_normal = mat3(u_model) * vs_in_normal;
    vs_out_uv = vs_in_uv;

    gl_Position = u_projection * u_view * world_position;
}
)";

static constexpr char* STATIC_MESH_FRAGMENT_SHADER = R"(#version 100

precision mediump float;

varying vec3 vs_out_world_position;
varying vec2 vs_out_uv;
varying vec3 vs_out_normal;

uniform sampler2D u_diffuse_map;
uniform vec3      u_light_direction;
uniform vec3      u_light_colour;
uniform vec3      u_ambient_light_colour;
uniform vec4      u_surface_tint;

void main()
{
    vec4 diffuse_surface_colour = texture2D(u_diffuse_map, vs_out_uv);

    vec3 frag_normal = normalize(vs_out_normal);
    vec3 to_light = normalize(-u_light_direction);
    float normal_dot_to_light = max(dot(frag_normal, to_light), 0.0);

    vec3 ambient_light_colour = u_ambient_light_colour;
    vec3 diffuse_light_colour = u_light_colour * normal_dot_to_light;

    vec3 combined_light_colour = ambient_light_colour + diffuse_light_colour;

    vec4 surface_colour = vec4(
        diffuse_surface_colour.rgb * combined_light_colour,
        diffuse_surface_colour.a
    ) * u_surface_tint;

    gl_FragColor = surface_colour;
}
)";

#endif // _STATIC_MESH_SHADER_H_
