#include "Re.h"

#include <iostream>
#include <vector>
#include <memory>

namespace Re {

DFANode* NodeManager::DFAFromNFA(NFANode* nfa) {
    DFANode* dfa = makeDFANode(nfa);
    makeDFATransitions(dfa);
    return dfa;
}

NFANode* NFAFromRe(const char* re) {
    return nullptr;
}

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

DFANode* NodeManager::makeDFANode(const std::vector<NFANode*>& nfaNodes) {
    DFANode dfaNode;
    for (auto* nfaNode : nfaNodes) {
        dfaNode.bypassEPS(nfaNode);
    }

    // create a DFA node if it doesn't exist
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
// struct ReParser;
// using RePreSP = std::unique_ptr<ReParser>;

// struct ReParser {
//     enum class Type {
//         concatenation,
//         alternation,
//         kleene_closure
//     } type;
//     uint32_t start;
//     uint32_t end;
//     std::vector<RePreSP> pres;

//     ReParser(const Type type, const uint32_t start, const uint32_t end)
//         : type(type), start(start), end(end)
//     {}

//     static RePreSP makeRePre(const Type type, const uint32_t start, const uint32_t end) {
//         return std::make_unique<ReParser>(type, start, end);
//     }
// };

// int32_t findRightParen(const char* re, uint32_t start) {
//     auto numParenthesis = 0u;
//     // TODO performance? One pass
//     auto end = start + 1;
//     while (re[end]) {
//         switch (re[end])
//         {
//         case LEFT_PAREN:
//             numParenthesis++;
//             break;
//         case RIGHT_PAREN:
//             if (numParenthesis == 0) {
//                 return end;
//             }
//             else {                                
//                 numParenthesis--;
//             }
//             break;
//         default:
//             break;
//         }
//         end++;
//     }
//     throw ParenthesisMatchingException(
//         "Missing right parenthesis: " + std::string(re + start));
// }

// RePreSP parseReToPre(const char* re, uint32_t start, uint32_t end) {
//     while (start < end and re[start]) {
//         const auto sym = re[start];
//         switch (sym)
//         {
//         case LEFT_PAREN: {
//             auto end = findRightParen(re, start);
//             auto rePre = ReParser::makeRePre(ReParser::Type::concatenation, start+1, end-1);
//             if (re[end+1] == KLEENE_STAR) {
//                 rePre->type = ReParser::Type::kleene_closure;
//             }
//             start = end;
//         }
//         case RIGHT_PAREN:
//             throw ParenthesisMatchingException(
//                 "Unexpected right parenthesis: " + std::string(re, start, end));
//             break;
//         }
//         start++;
//     }
// }

// DFANode* DFANode::NFAToDFA(const NFANode* nfa) {
//     // DFANode* start = makeNode<DFANode>();

//     start->NFANodes.insert(nfa);
//     start->bypassEPS(nfa);
//     start->bfsNFAToDFA();
//     return start;
// }

// NFANodeSP constructNFA(const char* re, uint32_t start, uint32_t end) {
//     auto startNode = NFANodeSP{new NFANode()};
//     while (start < end and re[start]) {
//         const auto sym = re[start];
//         switch (sym)
//         {
//         case LEFT_PAREN: {
//             auto rightParenIdx = findRightParen(re, start);
//             auto subNFA = constructNFA(re, start + 1, rightParenIdx - 1);
//             startNode->addTransition(EPS, false, subNFA);
//             start = rightParenIdx;
//             break;
//         }
//         case RIGHT_PAREN:
//             throw ParenthesisMatchingException(
//                 "Unexpected right parenthesis: " + std::string(re, start, end));
//             break;
//         case BAR:
//             break;
//         case KLEENE_STAR:
//             break;
//         default: {
//             auto subNFA = constructNFA(re, start + 1, end);
//             startNode->addTransition(sym, false, subNFA);
//         }
//         }
//         start++;
//     }
//     return startNode;
// }

// NFANodeSP constructNFA(const std::string& re) {
//     return constructNFA(re.c_str(), 0, re.size());
// }

} // namespace Re

int main(void) {
    // auto r1 = Re::NFANode::constructNFA("abcd");
    // auto r2 = Re::NFANode::constructNFA("()");
    // auto r3 = Re::NFANode::constructNFA("aa((aaaa)");
    
    return 0;
}
