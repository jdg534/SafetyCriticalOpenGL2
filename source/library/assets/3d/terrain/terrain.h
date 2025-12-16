#ifndef _TERRAIN_H_
#define _TERRAIN_H_

#include <string>

#include "../../asset.h"

class terrain : public asset
{

public:

	terrain() = delete;
	terrain(const std::string& name, const std::string& path, std::weak_ptr<const asset_manager> asset_manager);
	virtual ~terrain();

	void initialise() override;
	void shutdown() override;
	asset_type get_type() const override;
};

#endif // _TERRAIN_H_
