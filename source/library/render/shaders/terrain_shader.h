#ifndef _TERRAIN_SHADER_H_
#define _TERRAIN_SHADER_H_

static constexpr char* TERRAIN_VERTEX_SHADER = R"(
#version 330 core

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

layout(location = 0) in vec3 vs_in_position;
layout(location = 1) in vec2 vs_in_uv;
layout(location = 2) in vec3 vs_in_normal;

out vec3 vs_out_world_position;
out vec2 vs_out_uv;
out vec3 vs_out_normal;

void main()
{
    vec4 world_position = u_model * vec4(vs_in_position, 1.0);
    vs_out_world_position = world_position.xyz;
    vs_out_normal = mat3(transpose(inverse(u_model))) * vs_in_normal;
    vs_out_uv = vs_in_uv;

    gl_Position = u_projection * u_view * world_position;
}
)";

static constexpr char* TERRAIN_FRAGMENT_SHADER = R"(
#version 330 core

in vec3 vs_out_world_position;
in vec2 vs_out_uv;
in vec3 vs_out_normal;

uniform sampler2D u_splat_map;
uniform sampler2D u_red_channel_diffuse_map;
uniform sampler2D u_green_channel_diffuse_map;
uniform sampler2D u_blue_channel_diffuse_map;
uniform sampler2D u_alpha_channel_diffuse_map;

out vec4 fs_out_frag_color;

void main()
{
    vec4 weights = texture(u_splat_map, vs_out_uv);

    vec3 frag_normal = normalize(vs_out_normal); // remember values from the VS are interploated.
    vec3 to_light = normalize(-u_light_direction);
    float normal_dot_to_light = max(dot(frag_normal, to_light), 0.0);

    vec3 ambient_light_colour = u_ambient_light_colour;
    vec3 diffuse_light_colour = u_light_colour * normal_dot_to_light;

    vec3 combined_light_colour = ambient_light_colour + diffuse_light_colour;

    vec4 combined_diffuse_colour =
      u_red_channel_diffuse_map * weights.r +
      u_green_channel_diffuse_map * weights.g +
      u_blue_channel_diffuse_map * weights.b +
      u_alpha_channel_diffuse_map * weights.a;

    vec4 surface_colour = vec4(combined_diffuse_colour.rgb * combined_light_colour, combined_diffuse_colour.a);
    fs_out_frag_color = surface_colour;
}
)";

#endif // _TERRAIN_SHADER_H_
