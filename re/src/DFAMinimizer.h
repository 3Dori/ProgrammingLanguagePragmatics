#pragma once

#include "FA.h"

#include <cstddef>
#include <list>
#include <set>
#include <memory>
#include <vector>

namespace RE
{

class NodeManager;

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
    DFAMinimizer(std::unique_ptr<NodeManager>&);
    std::unique_ptr<DFA> minimize();

private:
    MergedDfaNode* makeMergedDfaNode(const bool);
    void splitMergedDfaNodes(const MergedDfaNode&, const char);
    char searchForAmbiguousSymbol(const MergedDfaNode&) const;
    std::unique_ptr<DFA> constructMinimizedDFA() const;
    void mergeTransitions(const MergedDfaNode&, DFA&) const;

    void addDeadState(
        std::vector<DFANodeFromNFA*>&,
        std::set<char>&); /* so that each state has an transition
                             for each input */
    void removeDeadState();
private:
    std::vector<int32_t> m_DFAToMergedDFA;

    std::map<int32_t, MergedDfaNode> m_mergedDfaNodes;  // std::map has fixed capacity
    int32_t m_mergedDfaNodesId = 0;

    DFANodeFromNFA m_deadStateDFA;
    DFANodeFromNFA* m_deadState = nullptr;
};

} // namespace RE
