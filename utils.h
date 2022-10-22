#ifndef GIT_STATS_UTILS_H
#define GIT_STATS_UTILS_H

#include <filesystem>
#include <iostream>
#if defined(HAS_CLANG_SOURCE_LOCATION) || defined(HAS_GCC)
#include <source_location>
#endif

#include <git2.h>

namespace {
#if defined(HAS_CLANG_SOURCE_LOCATION) || defined(HAS_GCC)
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
#else
    struct MySourceLocation {
        const std::string_view file_name;
        const std::string_view func_name;
        const int line_number;

        static constexpr MySourceLocation current(const char * file = __builtin_FILE(), int line=__builtin_LINE(), const char *func=__builtin_FUNCTION()) {
            return {
                .file_name=file,
                .func_name=func,
                .line_number=line,
            };
        }
    };

    inline void check_error(int err_code, const MySourceLocation loc = MySourceLocation::current()) noexcept {
        [[unlikely]] if (err_code >= 0) {
            return;
        }

        auto error = git_error_last();
        auto message = error ? error->message : "";
        std::cout << loc.file_name
                  << ":"
                  << loc.func_name
                  << ":"
                  << loc.line_number
                  << ":error code: " << err_code << " error message: " << message << "\n";
        std::exit(1);
    }
#endif

    inline bool is_git_repo(const std::filesystem::path &p) noexcept {
        return std::filesystem::exists(p / ".git");
    }
}

#endif //GIT_STATS_UTILS_H
