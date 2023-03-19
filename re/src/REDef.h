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
    ESCAPE_D = 'd',
    ESCAPE_N = 'n',
    ESCAPE_T = 't',
    ESCAPE_R = 'r',
};

using NFAStateSet = std::set<NFAState const*>;

constexpr auto MAX_BRACES_REPETITION = 1024u;

} // namespace RE
