#include "FA.h"
#include "REDef.h"

#include <RE.h>

namespace RE
{

// NFA
void NFAState::addTransition(const char sym, NFAState const* to) {
    m_transitions[sym].insert(to);
}

bool NFAState::hasTransition(const char sym) const {
    return m_transitions.find(sym) != m_transitions.end();
}

// DFA
bool DFAState::accept(REParser::Str_t str) const {
    DFAState const* state = this;
    for (const auto c : str) {
        if (not state->hasTransition(c)) {
            return false;
        }
        else {
            state = state->m_transitions.at(c);
        }
    }
    return state->m_isFinal;
}

void DFAState::addTransition(const char sym, DFAState const* to) {
    assert(sym != EPS);
    assert(m_transitions.find(sym) == m_transitions.end());
    m_transitions[sym] = to;
}

bool DFAState::hasTransition(const char sym) const {
    return m_transitions.find(sym) != m_transitions.end();
}

bool DFAStateFromNFA::hasState(NFAState const* nfaState) const {
    return m_NFAStateSet.find(nfaState) != m_NFAStateSet.end();
}

} // namespace RE
