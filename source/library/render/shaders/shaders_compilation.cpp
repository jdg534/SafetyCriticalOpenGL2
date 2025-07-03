#include "shaders_compilation.h"

#include "../include_opengl.h"

#include <iostream>

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

gl::GLuint shaders_compilation::link_shaders_to_program(gl::GLuint vertex_shader_id, gl::GLuint fragment_shader_id)
{
	gl::GLuint p = gl::glCreateProgram();
	gl::glAttachShader(p, vertex_shader_id);
	gl::glAttachShader(p, fragment_shader_id);
	gl::glLinkProgram(p);
	gl::GLint ok = 0;
	gl::glGetProgramiv(p, gl::GL_LINK_STATUS, &ok);
	if (!ok)
	{
		char log[512];
		std::memset(log, 0, 512);
		gl::glGetProgramInfoLog(p, 512, nullptr, log);
		std::cerr << "Program link error:\n" << log << std::endl;
		std::exit(EXIT_FAILURE);
	}
	return p;
}