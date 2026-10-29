#pragma once
#include <algorithm>
#include <memory>
#include <string>
#include <stdexcept>
#include <utility>

namespace lms {

enum class Action {
    ManageAccounts, ManageCourses, AssignLecturers, EnrolStudents,
    CreateAssignment, GrantExtension, MarkSubmission,
    ModerateMarks, ReleaseMarks, SubmitWork, ViewReleasedMarks
};

class User {
public:
    User(int id, std::string username, std::string fullName, int departmentId)
        : id_(id), username_(std::move(username)), fullName_(std::move(fullName)), departmentId_(departmentId) {}
    virtual ~User() = default;
    virtual std::string role() const = 0;
    virtual bool canPerform(Action action) const = 0;
    int id() const noexcept { return id_; }
    int departmentId() const noexcept { return departmentId_; }
    const std::string& username() const noexcept { return username_; }
    const std::string& fullName() const noexcept { return fullName_; }
private:
    int id_;
    std::string username_;
    std::string fullName_;
    int departmentId_;
};

class Administrator final : public User {
public:
    using User::User;
    std::string role() const override { return "admin"; }
    bool canPerform(Action a) const override {
        return a==Action::ManageAccounts || a==Action::ManageCourses ||
               a==Action::AssignLecturers || a==Action::EnrolStudents;
    }
};
class Lecturer final : public User {
public:
    using User::User;
    std::string role() const override { return "lecturer"; }
    bool canPerform(Action a) const override {
        return a==Action::CreateAssignment || a==Action::GrantExtension || a==Action::MarkSubmission;
    }
};
class HeadOfDepartment final : public User {
public:
    using User::User;
    std::string role() const override { return "hod"; }
    bool canPerform(Action a) const override { return a==Action::ModerateMarks || a==Action::ReleaseMarks; }
};
class Student final : public User {
public:
    using User::User;
    std::string role() const override { return "student"; }
    bool canPerform(Action a) const override { return a==Action::SubmitWork || a==Action::ViewReleasedMarks; }
};

class UserFactory {
public:
    static std::unique_ptr<User> create(const std::string& role, int id,
                                       const std::string& username, const std::string& fullName,
                                       int departmentId) {
        if (role=="admin") return std::make_unique<Administrator>(id,username,fullName,departmentId);
        if (role=="lecturer") return std::make_unique<Lecturer>(id,username,fullName,departmentId);
        if (role=="hod") return std::make_unique<HeadOfDepartment>(id,username,fullName,departmentId);
        if (role=="student") return std::make_unique<Student>(id,username,fullName,departmentId);
        throw std::invalid_argument("Unknown user role");
    }
};

class LatePenaltyStrategy {
public:
    virtual ~LatePenaltyStrategy() = default;
    virtual double finalMark(double rawMark, double maximumMark, int lateDays) const = 0;
};
class StandardLatePenalty final : public LatePenaltyStrategy {
public:
    double finalMark(double rawMark, double maximumMark, int lateDays) const override {
        if (maximumMark <= 0.0) throw std::invalid_argument("Maximum mark must be positive");
        if (lateDays > 3) return 0.0;
        const int days = std::max(0, lateDays);
        return std::min(maximumMark, std::max(0.0, rawMark - (0.10 * maximumMark * days)));
    }
};

}
