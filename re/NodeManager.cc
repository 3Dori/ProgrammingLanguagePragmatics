#include "NodeManager.h"
#include "re.h"

namespace Re {

NFANode* NodeManager::makeNFANode(const bool isFinal) {
    if (m_NFAs.size() >= MAX_NFA_NODE_NUM) {
        throw NFANumLimitExceededExpection("The limit of number of NFA nodes is exceeded");
    }
    m_NFAs.emplace_back(m_NFAs.size(), isFinal);
    return &(m_NFAs.back());
}

NodeManager::NFA NodeManager::makeSym(const char sym) {
    auto startNode = makeNFANode();
    auto endNode = makeNFANode(true);
    startNode->addTransition(sym, endNode);
    return {startNode, endNode};
}

NodeManager::NFA NodeManager::makeConcatenation(NFA& a, NFA& b) {
    assert(b.endNode->transitions.empty());
    a.endNode->m_isFinal = false;
    a.endNode->addTransition(EPS, b.startNode);
    return {a.startNode, b.endNode};
}

NodeManager::NFA NodeManager::makeAlternation(std::vector<NFA>& nodes) {
    auto startNode = makeNFANode();
    auto endNode = makeNFANode(true);

    for (auto& node : nodes) {
        node.endNode->m_isFinal = false;
        startNode->addTransition(EPS, node.startNode);
        node.endNode->addTransition(EPS, endNode);
    }

    return {startNode, endNode};
}

NodeManager::NFA NodeManager::makeKleeneClousure(NFA& nfa) {
    auto startNode = makeNFANode();
    auto endNode = makeNFANode(true);

    nfa.endNode->m_isFinal = false;

    startNode->addTransition(EPS, nfa.startNode);
    startNode->addTransition(EPS, endNode);
    nfa.endNode->addTransition(EPS, nfa.startNode);
    nfa.endNode->addTransition(EPS, endNode);

    return {startNode, endNode};
}

DFANode* NodeManager::makeDFANode(NFANode* nfaNode) {
    DFANode dfaNode;
    dfaNode.bypassEPS(nfaNode);
    return createDFANodeIfNotExists(dfaNode);
}

DFANode* NodeManager::makeDFANode(const std::vector<NFANode*>& nfaNodes) {
    DFANode dfaNode;
    for (auto* nfaNode : nfaNodes) {
        dfaNode.bypassEPS(nfaNode);
    }
    return createDFANodeIfNotExists(dfaNode);
}

DFANode* NodeManager::createDFANodeIfNotExists(const DFANode& dfaNode) {
    const auto NFANodesInvolved = dfaNode.m_NFANodes;
    if (not m_DFAs.contains(NFANodesInvolved)) {
        m_DFAs[NFANodesInvolved] = dfaNode;
    }
    else {
        assert(m_DFAs[NFANodesInvolved] == dfaNode);
    }
    return getDFANode(NFANodesInvolved);
}

DFANode* NodeManager::getDFANode(const NFASet& NFAs) {
    return &(m_DFAs[NFAs]);
}

void NodeManager::makeDFATransitions(DFANode* dfaNode) {
    for (const auto* nfaNode : dfaNode->m_NFANodeSet) {
        for (const auto& [sym, tos] : nfaNode->transitions) {
            if (sym != EPS and not dfaNode->hasTransition(sym)) {
                DFANode* nextDfaNode = makeDFANode(tos);
                dfaNode->addTransition(sym, nextDfaNode);
                makeDFATransitions(nextDfaNode);
            }
        }
    }
}

}