#pragma once

#include <set>

namespace RE {

class NFANode;

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

using NFANodeSet = std::set<NFANode const*>;

} // namespace RE
