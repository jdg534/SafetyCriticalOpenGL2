#ifndef _TERRAIN_SHADER_H_
#define _TERRAIN_SHADER_H_

// note we're only able to use #version 100 since the code needs to be glsl ES compilent.
static constexpr char* TERRAIN_VERTEX_SHADER = R"(#version 100

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

attribute vec3 vs_in_position;
attribute vec2 vs_in_uv;
attribute vec3 vs_in_normal;
attribute vec2 vs_in_terrain_uv;

varying vec3 vs_out_world_position;
varying vec2 vs_out_uv;
varying vec3 vs_out_normal;
varying vec2 vs_out_terrain_uv;

void main()
{
    vec4 world_position = u_model * vec4(vs_in_position, 1.0);
    vs_out_world_position = world_position.xyz;

    // No scaling assumption
    vs_out_normal = mat3(u_model) * vs_in_normal;

    vs_out_uv = vs_in_uv;
    vs_out_terrain_uv = vs_in_terrain_uv;

    gl_Position = u_projection * u_view * world_position;
}
)";

static constexpr char* TERRAIN_FRAGMENT_SHADER = R"(#version 100

precision mediump float;

uniform sampler2D u_splat_map;
uniform sampler2D u_red_channel_diffuse_map;
uniform sampler2D u_green_channel_diffuse_map;
uniform sampler2D u_blue_channel_diffuse_map;
uniform sampler2D u_alpha_channel_diffuse_map;

uniform vec3 u_light_direction;
uniform vec3 u_light_colour;
uniform vec3 u_ambient_light_colour;
uniform vec4 u_terrain_blend_colour;

varying vec3 vs_out_world_position;
varying vec2 vs_out_uv;
varying vec3 vs_out_normal;
varying vec2 vs_out_terrain_uv;

void main()
{
    vec4 weights = texture2D(u_splat_map, vs_out_uv);

    vec3 frag_normal = normalize(vs_out_normal);
    vec3 to_light = normalize(-u_light_direction);
    float normal_dot_to_light = max(dot(frag_normal, to_light), 0.0);

    vec3 ambient_light_colour = u_ambient_light_colour;
    vec3 diffuse_light_colour = u_light_colour * normal_dot_to_light;
    vec3 combined_light_colour = ambient_light_colour + diffuse_light_colour;

    vec4 combined_diffuse_colour =
        texture2D(u_red_channel_diffuse_map, vs_out_terrain_uv) * weights.r +
        texture2D(u_green_channel_diffuse_map, vs_out_terrain_uv) * weights.g +
        texture2D(u_blue_channel_diffuse_map, vs_out_terrain_uv) * weights.b +
        texture2D(u_alpha_channel_diffuse_map, vs_out_terrain_uv) * weights.a;

    vec4 surface_colour = vec4(
        combined_diffuse_colour.rgb * combined_light_colour,
        combined_diffuse_colour.a
    );

    gl_FragColor = surface_colour * u_terrain_blend_colour;
}
)";

#endif // _TERRAIN_SHADER_H_
