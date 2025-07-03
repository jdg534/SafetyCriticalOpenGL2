#pragma once

#include <glbinding/gl/types.h>

namespace shaders_compilation
{
	gl::GLuint compile_shader(gl::GLenum type, const char* src);
	gl::GLuint link_shaders_to_program(gl::GLuint vertex_shader_id, gl::GLuint fragment_shader_id);
}