#ifndef _ASSET_UTILS_H_
#define _ASSET_UTILS_H_

#include <string>
#include <string_view>
#include <filesystem>

namespace asset_utils
{
	std::string_view get_directory_path(std::string_view file_path);
	std::string resolve_file_path(std::string to_resolve, const std::filesystem::path& relative_to);
}

#endif // _ASSET_UTILS_H_
