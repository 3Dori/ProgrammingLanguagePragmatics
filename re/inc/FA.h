#pragma once

#include "ReUtility.h"

#include <cassert>
#include <vector>
#include <string_view>
#include <set>
#include <map>


namespace Re {

struct NFANode {
    NFANode(const size_t id, const bool isFinal) : m_id(id), m_isFinal(isFinal) {}

    NFANode(const NFANode&) = delete;
    NFANode& operator=(const NFANode&) = delete;

    const size_t m_id;
    bool m_isFinal;
    std::map<char, std::vector<NFANode const*>> transitions;

    void addTransition(const char sym, NFANode* to) {
        transitions[sym].push_back(to);
    }

    bool hasTransition(const char sym) const {
        return transitions.contains(sym);
    }
};

struct DFANode {
    bool m_isFinal = false;
    std::map<char, DFANode*> m_transitions;
    NFASet m_NFANodes;   // id of a DFA node
    std::set<NFANode const*> m_NFANodeSet;  // easy accessor to NFA nodes

    bool operator==(const DFANode& other) {
        return (m_isFinal == other.m_isFinal and
                m_NFANodes == other.m_NFANodes);
    }  // TODO remove and corresponding assert

    bool accept(std::string_view str) const {
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

    void bypassEPS(NFANode const* nfaNode) {
        if (nfaNode->m_isFinal) {
            m_isFinal = true;
        }
        m_NFANodes.set(nfaNode->m_id, true);
        m_NFANodeSet.insert(nfaNode);

        if (not nfaNode->hasTransition(EPS)) {
            return;
        }
        for (auto const* to : nfaNode->transitions.at(EPS)) {  // in a DFS manner
            if (not m_NFANodes[to->m_id]) {
                bypassEPS(to);
            }
        }
    }
};

} // namespace Re