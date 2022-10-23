#include <iostream>
#include <filesystem>
#include <ctime>
#include <functional>
#include <semaphore>
#include <vector>
#include <thread>
#include <atomic>

#include <git2.h>

#include "Repository.h"

#ifdef MEASURE_TIME
#include <chrono>
#endif

int main(int argc, const char *argv[]) {
    if (argc < 3) {
        std::cerr << "error: no enough arguments\n";
        std::exit(1);
    }
    if (!std::filesystem::exists(argv[1])) {
        std::cerr << "error: " << argv[1] << " does not exists\n";
        std::exit(1);
    }
    git_libgit2_init();
#ifdef MEASURE_TIME
    auto old_now = std::chrono::system_clock::now();
#endif
    std::tm start{};
    start.tm_sec=0;
    start.tm_min=0;
    start.tm_mday=1;
    start.tm_hour=0;
    start.tm_mon=1;
    start.tm_year=2021-1900;
    auto start_t = std::mktime(&start);
    std::filesystem::directory_iterator iter{argv[1], std::filesystem::directory_options::skip_permission_denied};
    std::atomic_size_t all_add = 0, all_del = 0;
    std::vector<std::thread> jobs;
    std::counting_semaphore sema{std::thread::hardware_concurrency()};
    for (const auto &p : iter) {
        if (!p.is_directory()) {
            continue;
        }
        if (!is_git_repo(p.path())) {
            continue;
        }
        sema.acquire();
        jobs.emplace_back(std::thread([argv, start_t, p = p.path(), &all_del, &all_add, &sema]{
            auto repo = Repository(p);
            auto [del, add] = repo.calc_author_contributions(argv[2], static_cast<int64_t>(start_t));
            all_add += add;
            all_del += del;
            sema.release();
        }));
    }
    for (auto &t : jobs) {
        t.join();
    }
    std::cout << "all projects add: " << all_add << "\n";
    std::cout << "all projects del: " << all_del << "\n";
    git_libgit2_shutdown();
#ifdef MEASURE_TIME
    auto now = std::chrono::system_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(now-old_now).count() << "ms\n";
#endif
    return 0;
}
