#include "Re.h"

#include <gtest/gtest.h>


TEST(ReTest, CanParseAndMatchBasicRe_1) {
    Re::ReParser parser("abcd");
    EXPECT_TRUE(parser.match("abcd"));
    EXPECT_TRUE(parser.match("aaaabcd"));
    EXPECT_TRUE(parser.match("abcababcd"));
    EXPECT_FALSE(parser.match("abbcd"));
}

TEST(ReTest, CanParseAndMatchBasicRe_2) {
    Re::ReParser parser("a");
    EXPECT_TRUE(parser.match("a"));
    EXPECT_TRUE(parser.match("AAAaa"));
    EXPECT_TRUE(parser.match("bbbbbbbbbbbbbba"));
    EXPECT_FALSE(parser.match("bbbbbbbbbbbbbb"));
}
