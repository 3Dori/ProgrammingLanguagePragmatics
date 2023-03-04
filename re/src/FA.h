#pragma once

#include "ReUtility.h"

#include <cassert>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <string>

namespace Re {

struct NFANode {
    NFANode(const size_t id, const bool isFinal) : m_id(id), m_isFinal(isFinal) {}

    size_t m_id;
    bool m_isFinal;
    std::unordered_map<char, std::vector<NFANode*>> transitions;

    void addTransition(const char sym, NFANode* to) {
        transitions[sym].push_back(to);
    }
};

struct DFANode {
    bool m_isFinal = false;
    std::unordered_map<char, DFANode*> m_transitions;
    NFASet m_NFANodes;   // id of a DFA node
    std::unordered_set<NFANode*> m_NFANodeSet;  // easy accessor to NFA nodes

    bool operator==(const DFANode& other) {
        return (m_isFinal == other.m_isFinal and
                m_NFANodes == other.m_NFANodes);
    }

    void addTransition(const char sym, DFANode* to) {
        assert(sym != EPS);
        m_transitions[sym] = to;
    }

    bool hasState(NFANode* nfaNode) const {
        return m_NFANodes[nfaNode->m_id];
    }

    bool hasTransition(const char sym) const {
        return m_transitions.contains(sym);
    }

    void bypassEPS(NFANode*);
};

} // namespace Re
