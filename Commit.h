#ifndef GIT_STATS_COMMIT_H
#define GIT_STATS_COMMIT_H

#include <iostream>
#include <memory>

#include <git2.h>

#include "utils.h"

class Commit {
    struct commit_deleter {
        void operator()(git_commit *c) noexcept {
            git_commit_free(c);
        }
    };

    using commit_ptr = std::unique_ptr<git_commit, commit_deleter>;
public:
    explicit Commit(git_commit *c): commit{c, commit_deleter{}} {}

    [[nodiscard]] std::string get_message() const {
        return {git_commit_message(commit.get())};
    }

    [[nodiscard]] std::string get_author() const {
        auto sig = git_commit_author(commit.get());
        return {sig->name};
    }

    [[nodiscard]] std::string get_email() const {
        auto sig = git_commit_author(commit.get());
        return {sig->email};
    }

    [[nodiscard]] int get_parent_count() const {
        return static_cast<int>(git_commit_parentcount(commit.get()));
    }

    [[nodiscard]] Commit get_parent(int index) const {
        git_commit *parent = nullptr;
        int err = git_commit_parent(&parent, commit.get(), index);
        check_error(err);
        return Commit(parent);
    }

    [[nodiscard]] git_tree *get_tree() const {
        git_tree *tree = nullptr;
        int err = git_commit_tree(&tree, commit.get());
        check_error(err);
        return tree;
    }

    bool operator==(const Commit &c) const {
        auto left_id = git_commit_tree_id(commit.get());
        auto right_id = git_commit_tree_id(c.commit.get());
        return git_oid_equal(left_id, right_id);
    }

    [[nodiscard]] bool after_date(const int64_t start) const {
        git_time_t t = git_commit_time(commit.get());
        return t >= static_cast<git_time_t>(start);
    }

    [[nodiscard]] bool is_merge_commit() const {
        // github or gitlab
        return get_message().find("Merge branch") != std::string::npos
               || get_message().find("Merge pull request") != std::string::npos;
    }

    [[nodiscard]] bool match_email(const std::string &email) const {
        return get_email().find(email) != std::string::npos;
    }

private:
    commit_ptr commit;
};

#endif //GIT_STATS_COMMIT_H
