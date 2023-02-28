#pragma once

#include <vector>
#include <unordered_map>
#include <bitset>

namespace Re {

struct NFANode;
struct DFANode;

struct NodeManager {
    static constexpr size_t MAX_NFA_NODE_NUM = 1024;
    using NFASet = std::bitset<MAX_NFA_NODE_NUM>;

    struct NFA {
        NFANode* startNode;
        NFANode* endNode;
    };

    NFANode* makeNFANode(const bool isFinal = false);
    NFA makeSym(const char sym);
    NFA makeConcatenation(NFA& a, NFA& b);
    NFA makeAlternation(std::vector<NFA>& nodes);
    NFA makeKleeneClousure(NFA& nfa);

    DFANode* makeDFANode(NFANode* nfaNode);
    DFANode* makeDFANode(const std::vector<NFANode*>& nfaNodes);
    DFANode* createDFANodeIfNotExists(const DFANode&);
    DFANode* getDFANode(const NFASet& NFAs);
    void makeDFATransitions(DFANode* dfaNode);

private:
    std::vector<NFANode> m_NFAs;
    std::unordered_map<NFASet, DFANode> m_DFAs;
};

}
