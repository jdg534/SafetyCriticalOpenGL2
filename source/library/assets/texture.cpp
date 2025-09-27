#include "texture.h"

#include <stdexcept>
#include <png.h>

// public
/////////


texture::texture(const std::string& name, std::weak_ptr<const asset_manager> asset_manager)
	: asset(name, asset_manager)
	, m_texture_id(-1)
	, m_width(-1)
	, m_height(-1)
{}

texture::~texture()
{
	// delete stuff
}

void texture::initialise(std::string_view file_path)
{
	if (file_path.find(".png") == std::string::npos)
	{
		throw std::exception("load texture called with non png texture.");
	}
	FILE* file = fopen(file_path.data(), "rb"); // path needs to be relevent to the assets list.
	if (!file)
	{
		throw std::exception("failed to load png file");
	}
	// Read header
	png_byte header[8];
	fread(header, 1, 8, file);
	if (png_sig_cmp(header, 0, 8)) {
		fclose(file);
		throw std::runtime_error(std::string("File is not a PNG: ") + file_path.data());
	}
	// Init png structures
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (!png_ptr) {
		fclose(file);
		throw std::runtime_error("png_create_read_struct failed");
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_read_struct(&png_ptr, nullptr, nullptr);
		fclose(file);
		throw std::runtime_error("png_create_info_struct failed");
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
		fclose(file);
		throw std::runtime_error("Error during PNG read");
	}

	png_init_io(png_ptr, file);
	png_set_sig_bytes(png_ptr, 8);
	png_read_info(png_ptr, info_ptr);

	m_width = png_get_image_width(png_ptr, info_ptr);
	m_height = png_get_image_height(png_ptr, info_ptr);
	const png_byte color_type = png_get_color_type(png_ptr, info_ptr);
	const png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

	// Convert palettes/gray/16-bit to RGBA8
	if (bit_depth == 16) { png_set_strip_16(png_ptr); }
	if (color_type == PNG_COLOR_TYPE_PALETTE) { png_set_palette_to_rgb(png_ptr); }
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) { png_set_expand_gray_1_2_4_to_8(png_ptr); }
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) { png_set_tRNS_to_alpha(png_ptr); }
	if (color_type == PNG_COLOR_TYPE_RGB ||
		color_type == PNG_COLOR_TYPE_GRAY ||
		color_type == PNG_COLOR_TYPE_PALETTE)
	{
		png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
	}

	if (color_type == PNG_COLOR_TYPE_GRAY ||
		color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
	{
		png_set_gray_to_rgb(png_ptr);
	}

	png_read_update_info(png_ptr, info_ptr);

	// Allocate pixel buffer
	std::vector<unsigned char> pixels(m_width * m_height * 4);
	std::vector<png_bytep> row_pointers(m_height);
	for (unsigned y = 0; y < m_height; ++y)
	{
		row_pointers[y] = pixels.data() + y * m_width * 4;
	}

	png_read_image(png_ptr, row_pointers.data());

	// Cleanup
	png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
	fclose(file);

	// gl code.
	
	gl::glGenTextures(1, &m_texture_id);
	gl::glBindTexture(gl::GL_TEXTURE_2D, m_texture_id);

	// Upload pixel data
	gl::glTexImage2D(
		gl::GL_TEXTURE_2D, 0, gl::GL_RGBA,
		m_width, m_height, 0,
		gl::GL_RGBA, gl::GL_UNSIGNED_BYTE,
		pixels.data()
	);

	// Generate mipmaps, SC ok?
	gl::glGenerateMipmap(gl::GL_TEXTURE_2D);

	// Texture parameters
	gl::glTexParameteri(gl::GL_TEXTURE_2D, gl::GL_TEXTURE_WRAP_S, gl::GL_REPEAT);
	gl::glTexParameteri(gl::GL_TEXTURE_2D, gl::GL_TEXTURE_WRAP_T, gl::GL_REPEAT);
	gl::glTexParameteri(gl::GL_TEXTURE_2D, gl::GL_TEXTURE_MIN_FILTER, gl::GL_LINEAR_MIPMAP_LINEAR);
	gl::glTexParameteri(gl::GL_TEXTURE_2D, gl::GL_TEXTURE_MAG_FILTER, gl::GL_LINEAR);
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
