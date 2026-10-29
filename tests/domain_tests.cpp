#include <gtest/gtest.h>
#include "lms/domain.hpp"

TEST(LatePenalty, OnTimeHasNoDeduction) {
    lms::StandardLatePenalty p;
    EXPECT_DOUBLE_EQ(p.finalMark(80,100,0),80);
}
TEST(LatePenalty, DeductsTenPercentOfMaximumPerLateDay) {
    lms::StandardLatePenalty p;
    EXPECT_DOUBLE_EQ(p.finalMark(80,100,1),70);
    EXPECT_DOUBLE_EQ(p.finalMark(80,100,2),60);
    EXPECT_DOUBLE_EQ(p.finalMark(80,100,3),50);
}
TEST(LatePenalty, MoreThanThreeDaysReceivesZero) {
    lms::StandardLatePenalty p;
    EXPECT_DOUBLE_EQ(p.finalMark(80,100,4),0);
}
TEST(LatePenalty, MarkCannotBeNegativeOrExceedMaximum) {
    lms::StandardLatePenalty p;
    EXPECT_DOUBLE_EQ(p.finalMark(5,100,1),0);
    EXPECT_DOUBLE_EQ(p.finalMark(120,100,0),100);
}
TEST(LatePenalty, InvalidMaximumThrows) {
    lms::StandardLatePenalty p;
    EXPECT_THROW(p.finalMark(10,0,0), std::invalid_argument);
}
TEST(UserFactory, CreatesRoleWithExpectedCapabilities) {
    auto lecturer = lms::UserFactory::create("lecturer",7,"teach","Teacher",1);
    EXPECT_EQ(lecturer->role(),"lecturer");
    EXPECT_TRUE(lecturer->canPerform(lms::Action::MarkSubmission));
    EXPECT_FALSE(lecturer->canPerform(lms::Action::ReleaseMarks));
}
TEST(UserFactory, RejectsUnknownRole) {
    EXPECT_THROW(lms::UserFactory::create("unknown",1,"x","X",1), std::invalid_argument);
}
