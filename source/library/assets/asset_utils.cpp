#include "asset_utils.h"

#include <string_view>

using namespace std;

string_view asset_utils::get_directory_path(string_view file_path)
{
    if (auto pos = file_path.find_last_of('/');
        pos != string_view::npos)
    {
        return file_path.substr(0, pos + 1);
    }
    return "./";
}

std::string asset_utils::resolve_file_path(std::string to_resolve, const std::filesystem::path& relative_to)
{
    // LLM generated code...
    // --- Detect path type ---
    if (to_resolve.empty()) return {};

    if (!to_resolve.empty() && to_resolve[0] == '*') { throw std::runtime_error("Unsupported wild card reference: " + to_resolve); }
    if (to_resolve.compare(0, 5, "data:") == 0) { throw std::runtime_error("Unsupported base64-encoded data URI: " + to_resolve.substr(0, 16) + "..."); }
    if (to_resolve.compare(0, 7, "http://") == 0 || to_resolve.compare(0, 8, "https://") == 0) { throw std::runtime_error("Unsupported network URLs: " + to_resolve); }

    // --- Normalize path separators ---
    replace(to_resolve.begin(), to_resolve.end(), '\\', '/');

    filesystem::path path_to_target_file(to_resolve);

    // --- Detect and resolve based on path type ---
    if (path_to_target_file.is_absolute()) { return path_to_target_file.lexically_normal().string(); }

    // Parent-relative or local-relative path
    bool isRelativeLike = (to_resolve.size() >= 2 && to_resolve[0] == '.' &&
        (to_resolve[1] == '/' || to_resolve[1] == '.')) || !path_to_target_file.has_root_path();

    if (isRelativeLike)
    {
        filesystem::path resolved = filesystem::weakly_canonical(relative_to / path_to_target_file);
        return resolved.string();
    }

    // If we somehow got something unknown
    throw std::runtime_error("Unknown or malformed path type: " + to_resolve);
}
