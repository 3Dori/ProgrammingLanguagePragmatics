#include <RE.h>
#include <REExceptions.h>

#include <gtest/gtest.h>

using ::testing::TestWithParam;
using ::testing::Values;


TEST(RETest, EscapeExceptions) {
    EXPECT_THROW(RE::REParser(R"(\)"), RE::EscapeException);
    EXPECT_THROW(RE::REParser(R"(\a)"), RE::EscapeException);
    EXPECT_THROW(RE::REParser(R"(a\1b)"), RE::EscapeException);
    EXPECT_THROW(RE::REParser(R"(\\\)"), RE::EscapeException);
    EXPECT_THROW(RE::REParser(R"(a\\\a)"), RE::EscapeException);
}

class RETestEscape : public TestWithParam<char> {
public:
    RE::REParser getParser() {
        return RE::REParser("\\" + getStr());
    }

    std::string getStr() {
        return std::string(1u, GetParam());
    }
};

TEST_P(RETestEscape, CanPArserAndMatchSingleEscapes) {
    EXPECT_TRUE(getParser().matchExact(getStr()));
}

INSTANTIATE_TEST_SUITE_P(TestEscape, RETestEscape,
                         Values('(', ')', '{', '}', '|', '*', '+', '?', '\\'));

TEST(RETest, CanParseAndMatchEscapes) {
    EXPECT_TRUE(RE::REParser(R"(\++)").matchExact("+"));
    EXPECT_TRUE(RE::REParser(R"(a*\++)").matchExact("aa+++"));
    EXPECT_FALSE(RE::REParser(R"(a*\++)").matchExact("aa"));

    EXPECT_TRUE(RE::REParser(R"(\**)").matchExact(""));
    EXPECT_TRUE(RE::REParser(R"(\**)").matchExact("****"));
    EXPECT_TRUE(RE::REParser(R"(ab\**c)").matchExact("ab*c"));
    EXPECT_FALSE(RE::REParser(R"(ab\**cc)").matchExact(R"(ab\*cc)"));

    EXPECT_TRUE(RE::REParser(R"(\+\|\\)").matchExact(R"(+|\)"));
    EXPECT_TRUE(RE::REParser(R"(\+-\*/%)").matchExact("+-*/%"));
}

TEST(RETest, CanParseAndMatchDigits_1) {
    RE::REParser parser(R"(\d)");
    EXPECT_TRUE(parser.matchExact("0"));
    EXPECT_TRUE(parser.matchExact("1"));
    EXPECT_TRUE(parser.matchExact("2"));
    EXPECT_TRUE(parser.matchExact("3"));
    EXPECT_TRUE(parser.matchExact("4"));
    EXPECT_TRUE(parser.matchExact("5"));
    EXPECT_TRUE(parser.matchExact("6"));
    EXPECT_TRUE(parser.matchExact("7"));
    EXPECT_TRUE(parser.matchExact("8"));
    EXPECT_TRUE(parser.matchExact("9"));

    EXPECT_FALSE(parser.matchExact("a"));
    EXPECT_FALSE(parser.matchExact("("));
    EXPECT_FALSE(parser.matchExact("\\d"));
    EXPECT_FALSE(parser.matchExact("d"));
    EXPECT_FALSE(parser.matchExact("12"));
}

TEST(RETest, CanParseAndMatchDigits_2) {
    RE::REParser parser(R"(\d*)");
    EXPECT_TRUE(parser.matchExact(""));
    EXPECT_TRUE(parser.matchExact("0"));
    EXPECT_TRUE(parser.matchExact("100"));
    EXPECT_TRUE(parser.matchExact("0123456789"));

    EXPECT_FALSE(parser.matchExact("0.5"));
    EXPECT_FALSE(parser.matchExact("0a1"));
}

TEST(RETest, CanParseAndMatchDigits_3) {  // TODO match real-life numerics
    RE::REParser parser(R"(-?\d+.?\d*)");  // TODO . is wildcard
    EXPECT_TRUE(parser.matchExact("0"));
    EXPECT_TRUE(parser.matchExact("0.3423"));
    EXPECT_TRUE(parser.matchExact("0000.3423"));
    EXPECT_TRUE(parser.matchExact("0."));
    EXPECT_TRUE(parser.matchExact("-1."));
    EXPECT_TRUE(parser.matchExact("3.1415926"));
    EXPECT_TRUE(parser.matchExact("0123456789"));

    EXPECT_FALSE(parser.matchExact("."));
    EXPECT_FALSE(parser.matchExact("0..3"));
    EXPECT_FALSE(parser.matchExact(".2"));
    EXPECT_FALSE(parser.matchExact("a.2"));
    EXPECT_FALSE(parser.matchExact("-.2"));
}