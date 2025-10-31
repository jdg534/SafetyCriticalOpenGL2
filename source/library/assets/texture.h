#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <string>
#include <string_view>

#include "asset.h"
#include "../render/include_opengl.h"

class texture : public asset
{
public:

	texture() = delete;
	texture(const std::string& name, std::weak_ptr<const asset_manager> asset_manager);
	virtual ~texture();

	void initialise(std::string_view file_path) override;
	void shutdown() override;
	asset_type get_type() const override;

	unsigned int get_width() const;
	unsigned int get_height() const;
	gl::GLuint get_id() const;

private:

	gl::GLuint m_texture_id;
	unsigned int m_width;
	unsigned int m_height;
};

#endif // _TEXTURE_H_
