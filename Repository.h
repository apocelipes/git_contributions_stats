#ifndef GIT_STATS_REPOSITORY_H
#define GIT_STATS_REPOSITORY_H

#include <string>
#include <iostream>
#include <filesystem>
#include <tuple>

#include <git2.h>

#include "Commit.h"

class Repository {
public:
    explicit Repository(const std::string &path) {
        int err = git_repository_open(&repo, path.c_str());
        check_error(err);
    }

    ~Repository() {
        git_repository_free(repo);
    }

    Repository(Repository&& r) noexcept {
        git_repository_free(repo);
        repo = r.repo;
        r.repo = nullptr;
    }
    Repository &operator=(Repository &&r) noexcept {
        git_repository_free(repo);
        repo = r.repo;
        r.repo = nullptr;
        return *this;
    }

    std::tuple<size_t, size_t>  calc_diff_lines(const Commit &oc, const Commit &nc) {
        if (oc == nc) {
            return {0, 0};
        }
        git_diff *diff;
        git_diff_options diff_opts = GIT_DIFF_OPTIONS_INIT;
        auto ot = oc.get_tree();
        auto nt = nc.get_tree();
        int err = git_diff_tree_to_tree(&diff, repo, ot, nt, &diff_opts);
        check_error(err);
        git_diff_stats *stats = nullptr;
        err = git_diff_get_stats(&stats, diff);
        check_error(err);
        auto del = git_diff_stats_deletions(stats);
        auto add = git_diff_stats_insertions(stats);
        git_diff_stats_free(stats);
        git_diff_free(diff);
        git_tree_free(ot);
        git_tree_free(nt);
        return {del, add};
    }

    std::tuple<size_t, size_t> calc_all_parent_diff(const std::string &email, const Commit &commit) {
        size_t ret_add = 0, ret_del = 0;
        for (unsigned int i = 0; i < commit.get_parent_count(); ++i) {
            auto parent = commit.get_parent(i);
            if (!parent.match_email(email)) {
                continue;
            }
            if (parent.is_merge_commit()) {
                continue;
            }
            auto [del, add] = calc_diff_lines(commit.get_parent(0), commit.get_parent(i));
            ret_add += add;
            ret_del += del;
        }
        return {ret_del, ret_add};
    }

    std::tuple<size_t, size_t> calc_author_contributions(const std::string &author_name, int64_t start) {
        using namespace std::string_literals;
        auto head = get_head_commit();
        size_t ret_add = 0, ret_del = 0;
        while (head.get_parent_count() != 0 && head.after_date(start)) {
            auto [del, add] = calc_all_parent_diff(author_name, head);
            ret_del += del;
            ret_add += add;
            head = head.get_parent(0);
        }
        // /path/to/repo/.git/
        auto name = std::filesystem::path(git_repository_commondir(repo)).parent_path().parent_path().stem().string();
        std::string output = "repo: "s
                             + name
                             + "\n\tall add: "s
                             + std::to_string(ret_add)
                             + "\n\tall del: "s
                             + std::to_string(ret_del)
                             + "\n";
        std::cout << output;
        return {ret_del, ret_add};
    }

private:
    git_repository *repo = nullptr;

    [[nodiscard]] Commit get_head_commit() const {
        git_commit *c = nullptr;
        git_oid id;
        git_reference_name_to_id(&id, repo, "HEAD");
        int err = git_commit_lookup(&c, repo, &id);
        check_error(err);
        return Commit{c};
    }
};

#endif //GIT_STATS_REPOSITORY_H
