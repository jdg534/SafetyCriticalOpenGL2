#pragma once

#include <string>
#include <string_view>

#include "asset_types.h"

class asset
{
public:

	asset() = delete;
	virtual ~asset();
	asset(const std::string& name);

	virtual void initialise(const std::string& file_path) = 0;
	virtual void shutdown() = 0;
	virtual asset_type get_type() const = 0;
	
	const std::string& get_name() const; // convert to string_view

private:
	std::string m_name;
};
