#ifndef _TERRAIN_H_
#define _TERRAIN_H_

#include "../../asset.h"
#include "../../../render/include_opengl.h"

#include <tiffio.h>

#include <string>
#include <vector>


class terrain : public asset
{

public:

	terrain() = delete;
	terrain(const std::string& name, const std::string& path, std::weak_ptr<const asset_manager> asset_manager);
	virtual ~terrain();

	void initialise() override;
	void shutdown() override;
	asset_type get_type() const override;

	uint16 get_tiff_width() const;
	uint16 get_tiff_length() const; // the differ from height, for y px in the image
	float get_tiff_meters_per_pixel() const;
	float get_tiff_height_at(uint16 x, uint16 y) const;
	float get_height_at(float x_world_space, float z_world_space) const;

	gl::GLuint get_vertex_array_object_id() const;
	gl::GLuint get_vertex_buffer_id() const;
	gl::GLuint get_index_buffer_id() const;
	gl::GLuint get_num_indices_to_draw() const;

	gl::GLuint get_splat_map_texture_id() const;
	gl::GLuint get_red_channel_mapped_texture_texture_id() const;
	gl::GLuint get_green_channel_mapped_texture_texture_id() const;
	gl::GLuint get_blue_channel_mapped_texture_texture_id() const;
	gl::GLuint get_alpha_channel_mapped_texture_texture_id() const;

private:

	static void read_heights_uint8(std::vector<float>& output_buffer, TIFF* tiff_file);
	static void read_heights_sint8(std::vector<float>& output_buffer, TIFF* tiff_file);
	static void flip_rows(std::vector<float>& output_buffer, uint32 width, uint32 length);

	size_t get_height_index(uint16 x, uint16 y) const;
	void generate_open_gl_buffers();
	void check_referenced_textures_are_valid();

	gl::GLuint get_texture_id(std::string_view texture_asset_name) const;

	std::vector<float> m_heights;
	uint16 m_tiff_width = 0;
	uint16 m_tiff_length = 0;
	float m_tiff_meters_per_pixel = 1.0f;

	gl::GLuint m_vertex_array_object_id = 0;
	gl::GLuint m_vertex_buffer_id = 0;
	gl::GLuint m_index_buffer_id = 0;
	gl::GLuint m_num_indices_to_draw = 0;

	std::string m_splat_map_asset_name;
	std::string m_red_channel_mapped_texture_asset_name;
	std::string m_green_channel_mapped_texture_asset_name;
	std::string m_blue_channel_mapped_texture_asset_name;
	std::string m_alpha_channel_mapped_texture_asset_name;
};

#endif // _TERRAIN_H_
