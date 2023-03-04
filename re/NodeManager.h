#pragma once

#include "ReUtility.h"
#include "FA.h"

#include <vector>
#include <unordered_map>

namespace Re {

/**
 * Manages the life cycle of NFA and DFA nodes
 */
struct NodeManager {
    struct NFA {
        NFANode* startNode;
        NFANode* endNode;
    };

private:
    NFANode* makeNFANode(const bool isFinal = false);
    NFA makeSym(const char sym);
    NFA makeConcatenation(NFA& a, NFA& b);
    NFA makeAlternation(std::vector<NFA>& nodes);
    NFA makeKleeneClousure(NFA& nfa);

    // DFANode* makeDFANode(NFANode* nfaNode);
    DFANode* makeDFANode(const std::vector<NFANode*>& nfaNodes);
    DFANode* getDFANode(const NFASet& NFAs);
    void makeDFATransitions(DFANode* dfaNode);

private:
    std::vector<NFANode> m_NFAs;
    std::unordered_map<NFASet, DFANode> m_DFAs;
};

}  // namespace Re
