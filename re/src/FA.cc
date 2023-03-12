#include "FA.h"
#include "REDef.h"

namespace RE
{

// NFA
void NFANode::addTransition(const char sym, NFANode const* to) {
    m_transitions[sym].insert(to);
}

bool NFANode::hasTransition(const char sym) const {
    return m_transitions.find(sym) != m_transitions.end();
}

// DFA
bool DFANode::accept(std::string_view str) const {
    DFANode const* node = this;
    for (const auto c : str) {
        if (not node->hasTransition(c)) {
            return false;
        }
        else {
            node = node->m_transitions.at(c);
        }
    }
    return node->m_isFinal;
}

void DFANode::addTransition(const char sym, DFANode const* to) {
    assert(sym != EPS);
    assert(m_transitions.find(sym) == m_transitions.end());
    m_transitions[sym] = to;
}

bool DFANode::hasTransition(const char sym) const {
    return m_transitions.find(sym) != m_transitions.end();
}

bool DFANodeFromNFA::hasState(NFANode const* nfaNode) const {
    return m_NFANodeSet.find(nfaNode) != m_NFANodeSet.end();
}

} // namespace RE
