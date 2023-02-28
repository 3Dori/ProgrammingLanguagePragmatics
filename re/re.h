#pragma once

#include "NodeManager.h"

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <exception>

namespace Re {

constexpr char EPS = 0;
constexpr char LEFT_PAREN = '(';
constexpr char RIGHT_PAREN = ')';
constexpr char BAR = '|';
constexpr char KLEENE_STAR = '*';

class ReException : public std::runtime_error {
public:
    explicit ReException(const std::string& message) : std::runtime_error(message) {}
};

class ParenthesisMatchingException : public ReException {
public:
    explicit ParenthesisMatchingException(const std::string& message) : ReException(message) {}
};

class NFANumLimitExceededExpection : public ReException {
public:
    explicit NFANumLimitExceededExpection(const std::string& message) : ReException(message) {}
};

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
    NodeManager::NFASet m_NFANodes;   // id of a DFA node
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

}