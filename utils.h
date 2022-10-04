#ifndef GIT_STATS_UTILS_H
#define GIT_STATS_UTILS_H

#include <filesystem>
#include <iostream>
#include <source_location>

#include <git2.h>

namespace {
    inline void check_error(int err_code, const std::source_location loc = std::source_location::current()) noexcept {
        [[unlikely]] if (err_code >= 0) {
            return;
        }

        auto error = git_error_last();
        std::cout << loc.file_name()
                  << ":"
                  << loc.function_name()
                  << ":"
                  << loc.line()
                  << ":error code: " << err_code << " error message: " << error->message << "\n";
        std::exit(1);
    }

    inline bool is_git_repo(const std::filesystem::path &p) noexcept {
        return std::filesystem::exists(p / ".git");
    }
}

#endif //GIT_STATS_UTILS_H
