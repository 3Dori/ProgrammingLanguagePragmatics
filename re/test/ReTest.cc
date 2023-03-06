#include "RE.h"

#include <gtest/gtest.h>


TEST(ReTest, CanParseAndMatchExactBasicSym_1) {
    Re::ReParser parser("abcd");
    EXPECT_TRUE(parser.matchExact("abcd"));
    EXPECT_FALSE(parser.matchExact("aaaabcd"));
    EXPECT_FALSE(parser.matchExact("abcababcd"));
    EXPECT_FALSE(parser.matchExact("abbcd"));
}

TEST(ReTest, CanParseAndMatchExactBasicSym_2) {
    Re::ReParser parser("a");
    EXPECT_TRUE(parser.matchExact("a"));
    EXPECT_FALSE(parser.matchExact("AAAaa"));
    EXPECT_FALSE(parser.matchExact("bbbbbbbbbbbbbba"));
    EXPECT_FALSE(parser.matchExact("bbbbbbbbbbbbbb"));
}

TEST(ReTest, CanParseAndMatchExactEmptyRe) {
    Re::ReParser parser("");
    EXPECT_TRUE(parser.matchExact(""));
    EXPECT_FALSE(parser.matchExact("aa"));
}

TEST(ReTest, KleeneStarExceptions) {
    EXPECT_THROW(Re::ReParser parser("1**"), Re::MultipleRepeatException);
    EXPECT_THROW(Re::ReParser parser("1+*"), Re::MultipleRepeatException);
    EXPECT_THROW(Re::ReParser parser("1*2**"), Re::MultipleRepeatException);
    EXPECT_THROW(Re::ReParser parser("1*2***"), Re::MultipleRepeatException);
    EXPECT_THROW(Re::ReParser parser("1*2+**"), Re::MultipleRepeatException);
    EXPECT_THROW(Re::ReParser parser("*123"), Re::NothingToRepeatException);
    EXPECT_THROW(Re::ReParser parser("*"), Re::NothingToRepeatException);
    EXPECT_THROW(Re::ReParser parser("**"), Re::NothingToRepeatException);
    // TODO more cases
}

TEST(ReTest, CanParseAndMatchExactKleeneStarForBasicSym_1) {
    Re::ReParser parser("1*");
    EXPECT_TRUE(parser.matchExact(""));
    EXPECT_TRUE(parser.matchExact("1"));
    EXPECT_TRUE(parser.matchExact("11"));
    EXPECT_TRUE(parser.matchExact("111111"));

    EXPECT_FALSE(parser.matchExact("2"));
    EXPECT_FALSE(parser.matchExact("211"));
    EXPECT_FALSE(parser.matchExact("1121"));
    EXPECT_FALSE(parser.matchExact("11a11111"));
}

TEST(ReTest, CanParseAndMatchExactKleeneStarForBasicSym_2) {
    Re::ReParser parser("1*ab*");
    EXPECT_TRUE(parser.matchExact("abb"));
    EXPECT_TRUE(parser.matchExact("11a"));
    EXPECT_TRUE(parser.matchExact("a"));
    EXPECT_TRUE(parser.matchExact("111abbbbbb"));

    EXPECT_FALSE(parser.matchExact("d"));
    EXPECT_FALSE(parser.matchExact("1ac"));
    EXPECT_FALSE(parser.matchExact("abc"));
    EXPECT_FALSE(parser.matchExact("aB"));
    EXPECT_FALSE(parser.matchExact("11babbbb"));
}

TEST(ReTest, PlusExceptions) {
    EXPECT_THROW(Re::ReParser parser("1++"), Re::MultipleRepeatException);
    EXPECT_THROW(Re::ReParser parser("1+2++"), Re::MultipleRepeatException);
    EXPECT_THROW(Re::ReParser parser("1?2*++"), Re::MultipleRepeatException);
    EXPECT_THROW(Re::ReParser parser("+123"), Re::NothingToRepeatException);
    EXPECT_THROW(Re::ReParser parser("+"), Re::NothingToRepeatException);
    EXPECT_THROW(Re::ReParser parser("++"), Re::NothingToRepeatException);
    // TODO more cases
}

TEST(ReTest, CanParseAndMatchExactPlus_1) {
    Re::ReParser parser("a+");
    EXPECT_TRUE(parser.matchExact("a"));
    EXPECT_TRUE(parser.matchExact("aa"));
    EXPECT_TRUE(parser.matchExact("aaaaaa"));

    EXPECT_FALSE(parser.matchExact(""));
    EXPECT_FALSE(parser.matchExact("aab"));
    EXPECT_FALSE(parser.matchExact("baa"));
}

TEST(ReTest, CanParseAndMatchExactPlus_2) {
    Re::ReParser parser("a+b+1");
    EXPECT_TRUE(parser.matchExact("ab1"));
    EXPECT_TRUE(parser.matchExact("aab1"));
    EXPECT_TRUE(parser.matchExact("abb1"));
    EXPECT_TRUE(parser.matchExact("aaaaaabbbbbbb1"));

    EXPECT_FALSE(parser.matchExact("1"));
    EXPECT_FALSE(parser.matchExact("c"));
    EXPECT_FALSE(parser.matchExact("a1"));
    EXPECT_FALSE(parser.matchExact("aaaaa1"));
    EXPECT_FALSE(parser.matchExact("b1"));
    EXPECT_FALSE(parser.matchExact("bbb1"));
    EXPECT_FALSE(parser.matchExact("a1231"));
    EXPECT_FALSE(parser.matchExact("123b1"));
}

TEST(ReTest, QuestionExceptions) {
    EXPECT_THROW(Re::ReParser parser("1??"), Re::MultipleRepeatException);
    EXPECT_THROW(Re::ReParser parser("1?2??"), Re::MultipleRepeatException);
    EXPECT_THROW(Re::ReParser parser("1?2+?*"), Re::MultipleRepeatException);
    EXPECT_THROW(Re::ReParser parser("?123"), Re::NothingToRepeatException);
    EXPECT_THROW(Re::ReParser parser("?"), Re::NothingToRepeatException);
    EXPECT_THROW(Re::ReParser parser("??"), Re::NothingToRepeatException);
    // TODO more cases
}

TEST(ReTest, CanParseAndMatchExactQuestion_1) {
    Re::ReParser parser("a?");
    EXPECT_TRUE(parser.matchExact(""));
    EXPECT_TRUE(parser.matchExact("a"));

    EXPECT_FALSE(parser.matchExact("b"));
    EXPECT_FALSE(parser.matchExact("aa"));
    EXPECT_FALSE(parser.matchExact("baaa"));
}

TEST(ReTest, CanParseAndMatchExactQuestion_2) {
    Re::ReParser parser("1a?b?");
    EXPECT_TRUE(parser.matchExact("1"));
    EXPECT_TRUE(parser.matchExact("1a"));
    EXPECT_TRUE(parser.matchExact("1b"));

    EXPECT_FALSE(parser.matchExact("11ab"));
    EXPECT_FALSE(parser.matchExact("1aab"));
    EXPECT_FALSE(parser.matchExact("1abbbb"));
    EXPECT_FALSE(parser.matchExact("1c"));
}