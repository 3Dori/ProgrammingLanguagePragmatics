#include "Re.h"

#include <gtest/gtest.h>


TEST(ReTest, CanParseAndMatchExactBasicRe_1) {
    Re::ReParser parser("abcd");
    EXPECT_TRUE(parser.matchExact("abcd"));
    EXPECT_FALSE(parser.matchExact("aaaabcd"));
    EXPECT_FALSE(parser.matchExact("abcababcd"));
    EXPECT_FALSE(parser.matchExact("abbcd"));
}

TEST(ReTest, CanParseAndMatchExactBasicRe_2) {
    Re::ReParser parser("a");
    EXPECT_TRUE(parser.matchExact("a"));
    EXPECT_FALSE(parser.matchExact("AAAaa"));
    EXPECT_FALSE(parser.matchExact("bbbbbbbbbbbbbba"));
    EXPECT_FALSE(parser.matchExact("bbbbbbbbbbbbbb"));
}
