#pragma once

static constexpr char* TEXTURED_QUAD_VERTEX_SHADER = R"(
#version 330 core

layout(location = 0) in vec2 vs_in_position_in_pixels;    // pixel coordinates (x, y)
layout(location = 1) in vec2 vs_in_uv;

uniform vec2 u_resolution; // viewport size in pixels (width, height)
uniform mat4 u_transform;  // optional 2D transform in pixel-space (affine). done as 4x4 to work with the class structure used.

out vec2 vs_out_uv;

void main()
{
    vec4 post_transform_px = u_transform * vec4(vs_in_position_in_pixels, 1.0, 1.0);
    vec2 normalised_device_coords = (post_transform_px.xy / u_resolution) * 2.0 - 1.0; // [0..res] -> [-1..1]
    normalised_device_coords.y = -normalised_device_coords.y; // flip Y (positive Y goes down)

    gl_Position = vec4(normalised_device_coords, 0.0, 1.0);
    vs_out_uv = vs_in_uv;
}
)";

static constexpr char* TEXTURED_QUAD_FRAGMENT_SHADER = R"(
#version 330 core

in vec2 vs_out_uv;

uniform sampler2D u_texture;
uniform vec4 u_tint;           // RGBA tint multiplier (use vec4(1.0) for no tint)
uniform float u_alpha_cut_off; // 0.0 = disabled. otherwise alpha values below are culled

out vec4 fs_out_frag_color;

void main()
{
    vec4 tex = texture(u_texture, vs_out_uv);
    if (u_alpha_cut_off > 0.0 && tex.a <= u_alpha_cut_off) discard;
    fs_out_frag_color = tex * u_tint;
}
)";
