#include "texture.h"

// public
/////////

texture::~texture()
{
	// delete stuff

}

texture::texture(const std::string& name, gl::GLuint texture_id, unsigned int width, unsigned int height)
	: asset(name)
	, m_texture_id(texture_id)
	, m_width(width)
	, m_height(height)
{}

void texture::initialise(const std::string& file_path)
{
	// nothing to do. need to refactor this
}

void texture::shutdown()
{
	gl::glDeleteTextures(1, &m_texture_id);
}

asset_type texture::get_type() const
{
	return asset_type::texture;
}

// private
//////////
