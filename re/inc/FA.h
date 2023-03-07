#pragma once

#include "REUtility.h"

#include <cassert>
#include <vector>
#include <string_view>
#include <set>
#include <map>


namespace RE {

class NFANode {
    friend class REParser;
    friend class NodeManager;
    friend class DFANode;

public:
    NFANode(const size_t, const bool);

private:
    NFANode(const NFANode&) = delete;
    NFANode& operator=(const NFANode&) = delete;

    void addTransition(const char, NFANode const*);
    bool hasTransition(const char) const;

    const size_t m_id;
    bool m_isFinal;
    std::map<char, std::set<NFANode const*>> m_transitions;
};

class DFANode {
    friend class REParser;
    friend class NodeManager;
    friend class NFANode;

public:
    DFANode() = default;
    DFANode& operator=(DFANode&&) = default;

private:
    DFANode(const DFANode&) = delete;
    DFANode& operator=(const DFANode&) = delete;
    DFANode(DFANode&&) = delete;

    bool accept(std::string_view) const;

    void addTransition(const char, DFANode*);
    bool hasState(NFANode const*) const;
    bool hasTransition(const char) const;

    void bypassEPS(NFANode const*);

    bool m_isFinal = false;
    std::map<char, DFANode*> m_transitions;
    NFASet m_NFANodes;   // id of a DFA node
    std::set<NFANode const*> m_NFANodeSet;  // easy accessor to NFA nodes
};

} // namespace RE
