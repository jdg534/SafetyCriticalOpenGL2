#include "texture.h"

// public
/////////


texture::texture(const std::string& name, std::weak_ptr<asset_manager> asset_manager, gl::GLuint texture_id, unsigned int width, unsigned int height)
	: asset(name, asset_manager)
	, m_texture_id(texture_id)
	, m_width(width)
	, m_height(height)
{}

texture::~texture()
{
	// delete stuff
}

void texture::initialise(const std::string& file_path)
{
	// refactor this... trigger an asset loader.
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
