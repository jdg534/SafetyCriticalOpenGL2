#pragma once // TODO convert to ifndef, define & endif. pragma once isn't in the c++ standard

#include <string>
#include <string_view>

#include "asset.h"
#include "../render/include_opengl.h"

// https://chatgpt.com/c/68ae29ce-4638-8325-b17a-f42ae5a7b74d we just want the struct to have the id and some data.

class texture : public asset
{
public:

	texture() = delete;
	texture(const std::string& name, std::weak_ptr<asset_manager> asset_manager, gl::GLuint texture_id, unsigned int width, unsigned int height);
	virtual ~texture();

	void initialise(const std::string& file_path) override;
	void shutdown() override;
	asset_type get_type() const override;

private:
	gl::GLuint m_texture_id;
	unsigned int m_width;
	unsigned int m_height;
};

