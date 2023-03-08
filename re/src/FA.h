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

    void mergeEPSTransition(NFANode const*);

    bool m_isFinal = false;
    std::map<char, DFANode*> m_transitions;
    NFASet m_NFANodes;   // id of a DFA node
    std::set<NFANode const*> m_NFANodeSet;  // easy accessor to NFA nodes
};

} // namespace RE
