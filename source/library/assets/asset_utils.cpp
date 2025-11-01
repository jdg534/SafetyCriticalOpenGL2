#include "asset_utils.h"

#include <string_view>

std::string_view asset_utils::get_directory_path(std::string_view file_path)
{
    if (auto pos = file_path.find_last_of('/');
        pos != std::string_view::npos)
    {
        return file_path.substr(0, pos + 1);
    }
    return "./";
}