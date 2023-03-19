#pragma once

#include <set>

namespace RE {

class NFAState;

enum ReservedSymbol {
    EPS = '\0',
    LEFT_PAREN = '(',
    RIGHT_PAREN = ')',
    LEFT_BRACE = '{',
    RIGHT_BRACE = '}',
    BAR = '|',
    KLEENE_STAR = '*',
    PLUS = '+',
    QUESTION = '?',
    ESCAPE = '\\',
    ESCAPE_d = 'd',
    ESCAPE_D = 'D',
    ESCAPE_n = 'n',
    ESCAPE_t = 't',
    ESCAPE_r = 'r',
};

using NFAStateSet = std::set<NFAState const*>;

constexpr auto MAX_BRACES_REPETITION = 1024u;

} // namespace RE
