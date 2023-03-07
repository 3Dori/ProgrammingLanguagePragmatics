#include "RE.h"

#include <gtest/gtest.h>


TEST(RETest, CanParseAndMatchExactBasicSym_1) {
    RE::REParser parser("a");
    EXPECT_TRUE(parser.matchExact("a"));
    EXPECT_FALSE(parser.matchExact("AAAaa"));
    EXPECT_FALSE(parser.matchExact("bbbbbbbbbbbbbba"));
    EXPECT_FALSE(parser.matchExact("bbbbbbbbbbbbbb"));
}

TEST(RETest, CanParseAndMatchExactBasicSym_2) {
    RE::REParser parser("abcd");
    EXPECT_TRUE(parser.matchExact("abcd"));
    EXPECT_FALSE(parser.matchExact("aaaabcd"));
    EXPECT_FALSE(parser.matchExact("abcababcd"));
    EXPECT_FALSE(parser.matchExact("abbcd"));
}

TEST(RETest, CanParseAndMatchExactEmptyRe) {
    RE::REParser parser("");
    EXPECT_TRUE(parser.matchExact(""));
    EXPECT_FALSE(parser.matchExact("aa"));
}

// TEST(RETest, ParenthesesExceptions) {
//     EXPECT_THROW(RE::REParser("("), MissingParenthsisException);
//     EXPECT_THROW(RE::REParser("a("), MissingParenthsisException);
//     EXPECT_THROW(RE::REParser("(a(b)c)("), MissingParenthsisException);
//     EXPECT_THROW(RE::REParser("(()"), MissingParenthsisException);

//     EXPECT_THROW(RE::REParser(")"), UnbalancedParenthesisException);
//     EXPECT_THROW(RE::REParser("a)"), UnbalancedParenthesisException);
//     EXPECT_THROW(RE::REParser("())"), UnbalancedParenthesisException);
//     EXPECT_THROW(RE::REParser("a(()())b())"), UnbalancedParenthesisException);
// }

// TEST(RETest, CanParseAndMatchExactBar_1) {
//     EXPECT_TRUE(RE::REParser("|").matchExact(""));
//     EXPECT_FALSE(RE::REParser("|").matchExact("|"));

//     RE::REParser parser("a|b");
//     EXPECT_TRUE(parser.matchExact("a"));
//     EXPECT_TRUE(parser.matchExact("b"));
//     EXPECT_FALSE(parser.matchExact("1"));
// }

// TEST(RETest, CanParseAndMatchExactBar_2) {
//     RE::REParser parser("ab|cd");
//     EXPECT_TRUE(parser.matchExact("ab"));
//     EXPECT_TRUE(parser.matchExact("cd"));
//     EXPECT_FALSE(parser.matchExact("abd"));
//     EXPECT_FALSE(parser.matchExact("acd"));
//     EXPECT_FALSE(parser.matchExact("bc"));
// }

TEST(RETest, KleeneStarExceptions) {
    EXPECT_THROW(RE::REParser parser("1**"), RE::MultipleRepeatException);
    EXPECT_THROW(RE::REParser parser("1+*"), RE::MultipleRepeatException);
    EXPECT_THROW(RE::REParser parser("1*2**"), RE::MultipleRepeatException);
    EXPECT_THROW(RE::REParser parser("1*2***"), RE::MultipleRepeatException);
    EXPECT_THROW(RE::REParser parser("1*2+**"), RE::MultipleRepeatException);
    EXPECT_THROW(RE::REParser parser("*123"), RE::NothingToRepeatException);
    EXPECT_THROW(RE::REParser parser("*"), RE::NothingToRepeatException);
    EXPECT_THROW(RE::REParser parser("**"), RE::NothingToRepeatException);
    // TODO more cases
    // EXPECT_THROW(RE::REParser parser("(*)"), RE::NothingToRepeatException);
}

TEST(RETest, CanParseAndMatchExactKleeneStarForBasicSym_1) {
    RE::REParser parser("1*");
    EXPECT_TRUE(parser.matchExact(""));
    EXPECT_TRUE(parser.matchExact("1"));
    EXPECT_TRUE(parser.matchExact("11"));
    EXPECT_TRUE(parser.matchExact("111111"));

    EXPECT_FALSE(parser.matchExact("2"));
    EXPECT_FALSE(parser.matchExact("211"));
    EXPECT_FALSE(parser.matchExact("1121"));
    EXPECT_FALSE(parser.matchExact("11a11111"));
}

TEST(RETest, CanParseAndMatchExactKleeneStarForBasicSym_2) {
    RE::REParser parser("1*ab*");
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

TEST(RETest, PlusExceptions) {
    EXPECT_THROW(RE::REParser parser("1++"), RE::MultipleRepeatException);
    EXPECT_THROW(RE::REParser parser("1+2++"), RE::MultipleRepeatException);
    EXPECT_THROW(RE::REParser parser("1?2*++"), RE::MultipleRepeatException);
    EXPECT_THROW(RE::REParser parser("+123"), RE::NothingToRepeatException);
    EXPECT_THROW(RE::REParser parser("+"), RE::NothingToRepeatException);
    EXPECT_THROW(RE::REParser parser("++"), RE::NothingToRepeatException);
    // TODO more cases
    // EXPECT_THROW(RE::REParser parser("(+)"), RE::NothingToRepeatException);
}

TEST(RETest, CanParseAndMatchExactPlus_1) {
    RE::REParser parser("a+");
    EXPECT_TRUE(parser.matchExact("a"));
    EXPECT_TRUE(parser.matchExact("aa"));
    EXPECT_TRUE(parser.matchExact("aaaaaa"));

    EXPECT_FALSE(parser.matchExact(""));
    EXPECT_FALSE(parser.matchExact("aab"));
    EXPECT_FALSE(parser.matchExact("baa"));
}

TEST(RETest, CanParseAndMatchExactPlus_2) {
    RE::REParser parser("a+b+1");
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

TEST(RETest, QuestionExceptions) {
    EXPECT_THROW(RE::REParser parser("1??"), RE::MultipleRepeatException);
    EXPECT_THROW(RE::REParser parser("1?2??"), RE::MultipleRepeatException);
    EXPECT_THROW(RE::REParser parser("1?2+?*"), RE::MultipleRepeatException);
    EXPECT_THROW(RE::REParser parser("?123"), RE::NothingToRepeatException);
    EXPECT_THROW(RE::REParser parser("?"), RE::NothingToRepeatException);
    EXPECT_THROW(RE::REParser parser("??"), RE::NothingToRepeatException);
    // TODO more cases
    // EXPECT_THROW(RE::REParser parser("(?)"), RE::NothingToRepeatException);
}

TEST(RETest, CanParseAndMatchExactQuestion_1) {
    RE::REParser parser("a?");
    EXPECT_TRUE(parser.matchExact(""));
    EXPECT_TRUE(parser.matchExact("a"));

    EXPECT_FALSE(parser.matchExact("b"));
    EXPECT_FALSE(parser.matchExact("aa"));
    EXPECT_FALSE(parser.matchExact("baaa"));
}

TEST(RETest, CanParseAndMatchExactQuestion_2) {
    RE::REParser parser("1a?b?");
    EXPECT_TRUE(parser.matchExact("1"));
    EXPECT_TRUE(parser.matchExact("1a"));
    EXPECT_TRUE(parser.matchExact("1b"));

    EXPECT_FALSE(parser.matchExact("11ab"));
    EXPECT_FALSE(parser.matchExact("1aab"));
    EXPECT_FALSE(parser.matchExact("1abbbb"));
    EXPECT_FALSE(parser.matchExact("1c"));
}

TEST(RETest, EscapeExceptions) {
    EXPECT_THROW(RE::REParser(R"(\)"), RE::EscapeException);
    EXPECT_THROW(RE::REParser(R"(\a)"), RE::EscapeException);
    EXPECT_THROW(RE::REParser(R"(a\1b)"), RE::EscapeException);
    EXPECT_THROW(RE::REParser(R"(\\\)"), RE::EscapeException);
    EXPECT_THROW(RE::REParser(R"(a\\\a)"), RE::EscapeException);
}

TEST(RETest, CanParseAndMatchEscapes) {
    EXPECT_TRUE(RE::REParser(R"(\\)").matchExact("\\"));
    EXPECT_TRUE(RE::REParser(R"(\+)").matchExact("+"));
    EXPECT_TRUE(RE::REParser(R"(\*)").matchExact("*"));
    EXPECT_TRUE(RE::REParser(R"(\?)").matchExact("?"));
    EXPECT_TRUE(RE::REParser(R"(\()").matchExact("("));
    EXPECT_TRUE(RE::REParser(R"(\))").matchExact(")"));
    EXPECT_TRUE(RE::REParser(R"(\|)").matchExact("|"));
    
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

TEST(RETest, CanParseAndMatchGeneralRE_1) {
    RE::REParser parser("a*bc+d?");
    EXPECT_TRUE(parser.matchExact("aaabccd"));
    EXPECT_TRUE(parser.matchExact("bcd"));
    EXPECT_TRUE(parser.matchExact("abc"));
    EXPECT_TRUE(parser.matchExact("bc"));

    EXPECT_FALSE(parser.matchExact("aaabbccd"));
    EXPECT_FALSE(parser.matchExact("aabccdd"));
    EXPECT_FALSE(parser.matchExact("1"));
    EXPECT_FALSE(parser.matchExact("aabd"));
    EXPECT_FALSE(parser.matchExact("bcdd"));
}