#pragma once

#include <set>

namespace RE {

class NFANode;

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
};

using NFANodeSet = std::set<NFANode const*>;

constexpr auto MAX_BRACES_REPETITION = 1024u;

} // namespace RE
