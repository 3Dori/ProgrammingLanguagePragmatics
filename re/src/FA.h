#pragma once

#include "REDef.h"

#include <cassert>
#include <bitset>
#include <vector>
#include <string_view>
#include <set>
#include <map>


namespace std {

// extends bitset's operator< to allow it to be used as key of std::map
template <size_t N>
bool operator<(const std::bitset<N>& lhs, const std::bitset<N>& rhs) noexcept {
    constexpr auto BITS_PER_BYTE = 8u;
    constexpr auto UNIT_SIZE = sizeof(unsigned long long) * BITS_PER_BYTE;
    constexpr std::bitset<N> ULL_MASK = 0ull - 1;
    constexpr auto maxLoop = (N - 1) / UNIT_SIZE + 1;
    for (auto shift = 0u; shift < maxLoop; ++shift) {
        // unit-wise (128 bits) comparison
        const auto lhsULL = ((lhs >> (UNIT_SIZE * shift)) & ULL_MASK).to_ullong();
        const auto rhsULL = ((rhs >> (UNIT_SIZE * shift)) & ULL_MASK).to_ullong();
        if (lhsULL < rhsULL) {
            return true;
        }
        else if (lhsULL > rhsULL) {
            return false;
        }
    }
    return false;  // lhs == rhs
}

} // namespace std


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
    std::map<char, std::set<NFANode const*>> m_transitions;
};

class DFANode {
    friend class DFAMinimizer;

public:
    // DFANode() = default;
    DFANode(const size_t id, const bool isFinal = false)
        : m_id(id), m_isFinal(isFinal) {}

    bool accept(std::string_view) const;

protected:
    void addTransition(const char, DFANode*);
    bool hasTransition(const char) const;

protected:
    size_t m_id;  // TODO: eliminate the need to use id
    bool m_isFinal = false;
    std::map<char, DFANode const*> m_transitions;  // TODO use const ptr
};

class DFANodeFromNFA : public DFANode {
    friend class REParser;
    friend class NodeManager;
    friend class NFANode;
    friend class DFAMinimizer;

public:
    // DFANodeFromNFA() = default;
    DFANodeFromNFA(const size_t id, const bool isFinal = false)
        : DFANode(id, isFinal) {}
    DFANodeFromNFA(DFANodeFromNFA&&) = default;

private:
    DFANodeFromNFA(const DFANodeFromNFA&) = delete;
    DFANodeFromNFA& operator=(const DFANodeFromNFA&) = delete;
    DFANodeFromNFA& operator=(DFANodeFromNFA&&) = delete;

    bool hasState(NFANode const*) const;

    void mergeEPSTransition(NFANode const*);

private:
    NodeSet m_NFANodes;   // id of a DFA node
    std::set<NFANode const*> m_NFANodeSet;  // easy accessor to NFA nodes
};


class DFA {    
    friend class DFAMinimizer;

public:
    bool accept(std::string_view str) const {
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
