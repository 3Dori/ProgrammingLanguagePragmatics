#pragma once

#include "REDef.h"

#include <cassert>
#include <vector>
#include <string_view>
#include <set>
#include <map>

namespace RE {

// TODO rename to states?
class NFANode {
    friend class REParser;
    friend class NodeManager;
    friend class DFANodeFromNFA;

public:
    explicit NFANode(const size_t id, const bool isFinal) :
        m_id(id), m_isFinal(isFinal)
    {}

private:
    NFANode(const NFANode&) = delete;
    NFANode& operator=(const NFANode&) = delete;

    void addTransition(const char, NFANode const*);
    bool hasTransition(const char) const;

    const size_t m_id;
    bool m_isFinal;
    std::map<char, NFANodeSet> m_transitions;
};

class DFANode {
    friend class DFAMinimizer;

public:
    DFANode(const size_t id, const bool isFinal = false)
        : m_id(id), m_isFinal(isFinal) {}

    DFANode(DFANode&&) = default;  // TODO avoid moving

    bool accept(std::string_view) const;

private:
    DFANode(const DFANode&) = delete;
    DFANode& operator=(const DFANode&) = delete;
    DFANode& operator=(DFANode&&) = delete;

protected:
    void addTransition(const char, DFANode const*);
    bool hasTransition(const char) const;

protected:
    size_t m_id;  // TODO: eliminate the need to use id
    bool m_isFinal = false;
    std::map<char, DFANode const*> m_transitions;
};

class DFANodeFromNFA : public DFANode {
    friend class REParser;
    friend class NodeManager;
    friend class NFANode;
    friend class DFAMinimizer;

public:
    DFANodeFromNFA(const size_t id, const bool isFinal = false)
        : DFANode(id, isFinal) {}
    DFANodeFromNFA(DFANodeFromNFA&&) = default;

private:
    bool hasState(NFANode const*) const;

    void mergeEPSTransition(NFANode const*);

private:
    NFANodeSet m_NFANodeSet;
};


class DFA {    
    friend class DFAMinimizer;

public:
    inline bool accept(std::string_view str) const {
        return m_start->accept(str);
    }

private:
    void setStart(const int32_t start) {
        m_start = &(m_nodes.at(start));
    }

private:
    std::map<int32_t, DFANode> m_nodes;  // actual storage
    DFANode const* m_start;
};

} // namespace RE
