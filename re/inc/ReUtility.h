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


namespace Re {

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

class ReException : public std::runtime_error {
public:
    explicit ReException(const std::string& message) : std::runtime_error(message) {}
};

class ParenthesisMatchingException : public ReException {
public:
    explicit ParenthesisMatchingException(const std::string& message) : ReException(message) {}
};

class NFANumLimitExceededExpection : public ReException {
public:
    explicit NFANumLimitExceededExpection() :
        ReException("The limit of number of NFA nodes is exceeded")
    {}
};

class MultipleRepeatException : public ReException {
public:
    explicit MultipleRepeatException(const size_t pos) :
        ReException("Multiple repeat at position " + std::to_string(pos))
    {}
};

class NothingToRepeatException : public ReException {
public:
    explicit NothingToRepeatException(const size_t pos) :
        ReException("Nothing to repeat at position " + std::to_string(pos))
    {}
};

} // namespace Re
