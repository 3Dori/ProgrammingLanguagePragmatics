#pragma once

#include "FA.h"

#include <cstddef>
#include <list>
#include <set>
#include <memory>
#include <vector>

namespace RE
{

class DFAMinimizer {
public:
    struct MergedDfaNode {
        MergedDfaNode(const int32_t id, const bool isFinal)
            : id(id), isFinal(isFinal) {}
        int32_t id;
        bool isFinal;
        std::set<DFANodeFromNFA const*> dfaNodes;
    };
    using MergedDfaNodes_it = std::list<MergedDfaNode>::iterator;

public:
    DFAMinimizer(const std::vector<DFANodeFromNFA const*>&);
    std::unique_ptr<DFA> minimize();

private:
    MergedDfaNode* makeMergedDfaNode(const bool);
    void splitMergedDfaNodes(const MergedDfaNode&, const char);
    char searchForAmbiguousSymbol(const MergedDfaNode&) const;
    void removeDeadState();
    std::unique_ptr<DFA> constructMinimizedDFA() const;
    void mergeTransitions(const MergedDfaNode&, DFA&) const;

private:
    std::vector<int32_t> m_DFAToMergedDFA;

    std::map<int32_t, MergedDfaNode> m_mergedDfaNodes;  // fixed capacity
    int32_t m_mergedDfaNodesId = 0;
    DFANodeFromNFA const* m_deadState;
};

} // namespace RE
