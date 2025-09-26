#pragma once // not c++ standard

#include <string>
#include <string_view>

namespace asset_utils
{
	std::string_view get_directory_path(std::string_view file_path);
}
