#ifndef _TERRAIN_H_
#define _TERRAIN_H_

#include "../../asset.h"

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

private:

	static void read_heights_uint8(std::vector<float>& output_buffer, TIFF* tiff_file);
	static void read_heights_sint8(std::vector<float>& output_buffer, TIFF* tiff_file);

	std::vector<float> m_heights;
};

#endif // _TERRAIN_H_
