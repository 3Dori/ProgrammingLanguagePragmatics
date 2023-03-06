#pragma once

#include <stdexcept>
#include <bitset>
#include <string>


namespace std {

// extends bitset's operator< to allow it to be the key of std::map
template <size_t N>
bool operator<(const std::bitset<N>& lhs, const std::bitset<N>& rhs) noexcept {
    constexpr auto BITS_PER_BYTE = 8u;
    constexpr auto UNIT_SIZE = sizeof(unsigned long long) * BITS_PER_BYTE;
    constexpr std::bitset<N> ULL_MASK = 0ull - 1;
    constexpr auto maxLoop = (N - 1) / UNIT_SIZE + 1;
    for (auto shift = 0u; shift < maxLoop; ++shift) {
        const auto lhsULL = ((lhs >> (UNIT_SIZE * shift)) & ULL_MASK).to_ullong();
        const auto rhsULL = ((rhs >> (UNIT_SIZE * shift)) & ULL_MASK).to_ullong();
        if (lhsULL < rhsULL) {
            return true;
        }
        else if (lhsULL > rhsULL) {
            return false;
        }
    }
    return false;
}

} // namespace std


namespace RE {

constexpr char EPS = 0;
constexpr char LEFT_PAREN = '(';
constexpr char RIGHT_PAREN = ')';
constexpr char BAR = '|';
constexpr char KLEENE_STAR = '*';
constexpr char PLUS = '+';
constexpr char QUESTION = '?';
constexpr char ESCAPE = '\\';

constexpr size_t MAX_NFA_NODE_NUM = 1024;
using NFASet = std::bitset<MAX_NFA_NODE_NUM>;

class REException : public std::runtime_error {
public:
    explicit REException(const std::string& message) : std::runtime_error(message) {}
};

class ParenthesisMatchingException : public REException {
public:
    explicit ParenthesisMatchingException(const std::string& message) : REException(message) {}
};

class NFANumLimitExceededExpection : public REException {
public:
    explicit NFANumLimitExceededExpection() :
        REException("The limit of number of NFA nodes is exceeded")
    {}
};

class MultipleRepeatException : public REException {
public:
    explicit MultipleRepeatException(const size_t pos) :
        REException("Multiple repeat at position " + std::to_string(pos))
    {}
};

class NothingToRepeatException : public REException {
public:
    explicit NothingToRepeatException(const size_t pos) :
        REException("Nothing to repeat at position " + std::to_string(pos))
    {}
};

class EscapeException : public REException {
public:
    explicit EscapeException(const char sym, const size_t pos) :
        REException(std::string("Unexpected escape character [") + sym + "] at position " + std::to_string(pos))
    {}

    explicit EscapeException(const std::string& message) :
        REException(message)
    {}
};

} // namespace RE
