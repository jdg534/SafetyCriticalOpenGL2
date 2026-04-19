#ifndef _SHADER_COMPILATION_H_
#define _SHADER_COMPILATION_H_

#include <glbinding/gl/types.h>
#include <vector>
#include <string_view>

namespace shaders_compilation
{
	gl::GLuint compile_shader(gl::GLenum type, const char* src);
	gl::GLuint link_shaders_to_program(gl::GLuint vertex_shader_id, gl::GLuint fragment_shader_id, const std::vector<std::string_view>& vertex_attributes);
}

#endif // _SHADER_COMPILATION_H_
