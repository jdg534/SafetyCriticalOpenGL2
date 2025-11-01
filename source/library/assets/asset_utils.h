#ifndef _ASSET_UTILS_H_
#define _ASSET_UTILS_H_

#include <string>
#include <string_view>

namespace asset_utils
{
	std::string_view get_directory_path(std::string_view file_path);
}

#endif // _ASSET_UTILS_H_
