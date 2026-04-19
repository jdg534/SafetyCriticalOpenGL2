#ifndef _TEXTURED_QUAD_SHADER_H_
#define _TEXTURED_QUAD_SHADER_H_

static constexpr char* TEXTURED_QUAD_VERTEX_SHADER = R"(#version 100

attribute vec2 vs_in_position_in_pixels;
attribute vec2 vs_in_uv;

uniform vec2 u_resolution;
uniform mat4 u_transform;

varying vec2 vs_out_uv;

void main()
{
    vec4 post_transform_px = u_transform * vec4(vs_in_position_in_pixels, 1.0, 1.0);
    vec2 normalised_device_coords = (post_transform_px.xy / u_resolution) * 2.0 - 1.0;
    normalised_device_coords.y = -normalised_device_coords.y;

    gl_Position = vec4(normalised_device_coords, 0.0, 1.0);
    vs_out_uv = vs_in_uv;
}
)";

static constexpr char* TEXTURED_QUAD_FRAGMENT_SHADER = R"(#version 100

precision mediump float;

varying vec2 vs_out_uv;

uniform sampler2D u_texture;
uniform vec4 u_tint;
uniform float u_alpha_cut_off;

void main()
{
    vec4 tex = texture2D(u_texture, vs_out_uv);

    if (u_alpha_cut_off > 0.0 && tex.a <= u_alpha_cut_off)
        discard;

    gl_FragColor = tex * u_tint;
}
)";

#endif // _TEXTURED_QUAD_SHADER_H_
