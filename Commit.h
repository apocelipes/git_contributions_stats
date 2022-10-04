#ifndef GIT_STATS_COMMIT_H
#define GIT_STATS_COMMIT_H

#include <iostream>

#include <git2.h>

#include "utils.h"

class Commit {
public:
    explicit Commit(git_commit *c): commit{c} {}
    Commit(const Commit& c) = delete;
    Commit(Commit &&c) noexcept {
        git_commit_free(commit);
        commit = c.commit;
        c.commit = nullptr;
    }

    Commit& operator=(Commit&& c) noexcept {
        git_commit_free(commit);
        commit = c.commit;
        c.commit = nullptr;
        return *this;
    }

    ~Commit() noexcept {
        git_commit_free(commit);
    }

    [[nodiscard]] std::string get_message() const {
        return {git_commit_message(commit)};
    }

    [[nodiscard]] std::string get_author() const {
        auto sig = git_commit_author(commit);
        return {sig->name};
    }

    [[nodiscard]] std::string get_email() const {
        auto sig = git_commit_author(commit);
        return {sig->email};
    }

    [[nodiscard]] unsigned int get_parent_count() const {
        return git_commit_parentcount(commit);
    }

    [[nodiscard]] Commit get_parent(int index) const {
        git_commit *parent = nullptr;
        int err = git_commit_parent(&parent, commit, index);
        check_error(err);
        return Commit(parent);
    }

    [[nodiscard]] git_tree *get_tree() const {
        git_tree *tree = nullptr;
        int err = git_commit_tree(&tree, commit);
        check_error(err);
        return tree;
    }

    bool operator==(const Commit &c) const {
        auto left_id = git_commit_tree_id(commit);
        auto right_id = git_commit_tree_id(c.commit);
        return git_oid_equal(left_id, right_id);
    }

    [[nodiscard]] bool after_date(const int64_t start) const {
        git_time_t t = git_commit_time(commit);
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
    git_commit *commit = nullptr;
};

#endif //GIT_STATS_COMMIT_H
