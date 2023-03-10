#pragma once

#include <bitset>


namespace RE {

enum ReservedSymbol {
    EPS = '\0',
    LEFT_PAREN = '(',
    RIGHT_PAREN = ')',
    BAR = '|',
    KLEENE_STAR = '*',
    PLUS = '+',
    QUESTION = '?',
    ESCAPE = '\\',
};

constexpr size_t MAX_NFA_NODE_NUM = 1024;
using NodeSet = std::bitset<MAX_NFA_NODE_NUM>;

} // namespace RE
