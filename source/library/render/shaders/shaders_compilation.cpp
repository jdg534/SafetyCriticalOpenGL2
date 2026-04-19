#include "shaders_compilation.h"

#include "../include_opengl.h"

#include <iostream>
#include <vector>

gl::GLuint shaders_compilation::compile_shader(gl::GLenum type, const char* src)
{
	gl::GLuint s = gl::glCreateShader(type);
	gl::glShaderSource(s, 1, &src, nullptr);
	gl::glCompileShader(s);
	gl::GLint ok = 0;
	gl::glGetShaderiv(s, gl::GL_COMPILE_STATUS, &ok);
	if (!ok)
	{
		char log[512];
		std::memset(log, 0, 512);
		gl::glGetShaderInfoLog(s, 512, nullptr, log);
		std::cerr << "Shader compile error:\n" << log << std::endl;
		std::exit(EXIT_FAILURE);
	}
	return s;
}

gl::GLuint shaders_compilation::link_shaders_to_program(gl::GLuint vertex_shader_id, gl::GLuint fragment_shader_id, const std::vector<std::string_view>& vertex_attributes)
{
	gl::GLuint shader_program = gl::glCreateProgram();
	for (size_t i = 0; i < vertex_attributes.size(); ++i)
	{
		gl::glBindAttribLocation(shader_program, static_cast<gl::GLuint>(i), vertex_attributes[i].data());
	}
	gl::glAttachShader(shader_program, vertex_shader_id);
	gl::glAttachShader(shader_program, fragment_shader_id);
	gl::glLinkProgram(shader_program);
	gl::GLint ok = 0;
	gl::glGetProgramiv(shader_program, gl::GL_LINK_STATUS, &ok);
	if (!ok)
	{
		char log[512];
		std::memset(log, 0, 512);
		gl::glGetProgramInfoLog(shader_program, 512, nullptr, log);
		std::cerr << "Program link error:\n" << log << std::endl;
		std::exit(EXIT_FAILURE);
	}
	return shader_program;
}