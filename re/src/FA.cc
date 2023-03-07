#include "FA.h"

namespace RE
{

// NFA
NFANode::NFANode(const size_t id, const bool isFinal) :
    m_id(id), m_isFinal(isFinal)
{}

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

void DFANode::addTransition(const char sym, DFANode* to) {
    assert(sym != EPS);
    m_transitions[sym] = to;
}

bool DFANode::hasState(NFANode const* nfaNode) const {
    return m_NFANodes[nfaNode->m_id];
}

bool DFANode::hasTransition(const char sym) const {
    return m_transitions.find(sym) != m_transitions.end();
}

void DFANode::bypassEPS(NFANode const* nfaNode) {
    if (nfaNode->m_isFinal) {
        m_isFinal = true;
    }
    m_NFANodes.set(nfaNode->m_id, true);
    m_NFANodeSet.insert(nfaNode);

    if (not nfaNode->hasTransition(EPS)) {
        return;
    }
    for (auto const* to : nfaNode->m_transitions.at(EPS)) {  // in a DFS manner
        if (not m_NFANodes[to->m_id]) {
            bypassEPS(to);
        }
    }
}

} // namespace RE
