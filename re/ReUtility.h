#pragma once

#include <stdexcept>
#include <bitset>

namespace Re {

constexpr char EPS = 0;
constexpr char LEFT_PAREN = '(';
constexpr char RIGHT_PAREN = ')';
constexpr char BAR = '|';
constexpr char KLEENE_STAR = '*';

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
    explicit NFANumLimitExceededExpection(const std::string& message) : ReException(message) {}
};

}  // namespace Re
