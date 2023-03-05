#include "Re.h"

#include <iostream>
#include <numeric>
#include <string_view>
#include <functional>
#include <vector>

// using std::placeholders::_1;
// using std::placeholders::_2;  TODO remove

namespace Re {

// NFA
NFANode* NodeManager::NFAFromRe(std::string_view re) {
    std::vector<NFA> nfas;
    for (auto pos = 0u; pos < re.size(); ++pos) {
        const auto c = re[pos];
        switch (c)
        {
        case EPS:
        case LEFT_PAREN:
        case RIGHT_PAREN:
        case BAR:
            assert(false and "Unimplemented");
        case KLEENE_STAR: {
            if (nfas.size() == 0) {
                throw NothingToRepeatException(pos);
            }
            NFA lastNfa = nfas.back();
            if (lastNfa.type == NFA::Type::kleene_star) {
                throw MultipleRepeatException(pos);
            }
            nfas.pop_back();
            nfas.push_back(makeKleeneClousure(lastNfa));
            break;
        }
        default:
            nfas.push_back(makeSymol(c));
            break;
        }
    }
    // TODO investigate it
    // NFA emptyNfa;
    // NFA nfa = std::accumulate(nfas.begin(), nfas.end(), emptyNfa,
                            //   std::bind(&NodeManager::makeConcatenation, this, _1, _2));
                            //   [this](NFA& a, NFA& b) {
                            //       return makeConcatenation(a, b);
                            //   });
    NFA resultNfa;
    for (auto& nfa : nfas) {
        resultNfa = makeConcatenation(resultNfa, nfa);
    }
    if (resultNfa.startNode == nullptr) {
        return makeNFANode(true);
    }
    return resultNfa.startNode;
}

NFANode* NodeManager::makeNFANode(const bool isFinal) {
    if (m_NFAs.size() >= MAX_NFA_NODE_NUM) {
        throw NFANumLimitExceededExpection();
    }
    m_NFAs.emplace_back(m_NFAs.size(), isFinal);
    assert(m_NFAs.back().m_id == m_NFAs.size() - 1 and "Wrong NFANode id");
    return &(m_NFAs.back());
}

NodeManager::NFA NodeManager::makeSymol(const char sym) {
    auto startNode = makeNFANode();
    auto endNode = makeNFANode(true);
    startNode->addTransition(sym, endNode);
    return {startNode, endNode, NFA::Type::symbol};
}

NodeManager::NFA NodeManager::makeConcatenation(NFA& a, NFA& b) {
    if (a.startNode == nullptr) {
        return b;
    }
    assert(b.endNode->transitions.empty());
    a.endNode->m_isFinal = false;
    a.endNode->addTransition(EPS, b.startNode);
    return {a.startNode, b.endNode, NFA::Type::concatenation};
}

NodeManager::NFA NodeManager::makeAlternation(std::vector<NFA>& nodes) {
    auto startNode = makeNFANode();
    auto endNode = makeNFANode(true);

    for (auto& node : nodes) {
        node.endNode->m_isFinal = false;
        startNode->addTransition(EPS, node.startNode);
        node.endNode->addTransition(EPS, endNode);
    }

    return {startNode, endNode, NFA::Type::alternation};
}

NodeManager::NFA NodeManager::makeKleeneClousure(NFA& nfa) {
    auto startNode = makeNFANode();
    auto endNode = makeNFANode(true);

    nfa.endNode->m_isFinal = false;

    startNode->addTransition(EPS, nfa.startNode);
    startNode->addTransition(EPS, endNode);
    nfa.endNode->addTransition(EPS, nfa.startNode);
    nfa.endNode->addTransition(EPS, endNode);

    return {startNode, endNode, NFA::Type::kleene_star};
}

// DFA
DFANode* NodeManager::DFAFromNFA(NFANode* nfa) {
    DFANode* dfa = makeDFANode(nfa);
    makeDFATransitions(dfa);
    return dfa;
}

DFANode* NodeManager::makeDFANode(const std::vector<NFANode const*>& nfaNodes) {
    DFANode dfaNode;
    for (auto const* nfaNode : nfaNodes) {
        dfaNode.bypassEPS(nfaNode);
    }

    // create a DFA node if it doesn't exist
    const auto NFANodesInvolved = dfaNode.m_NFANodes;
    if (not m_DFAs.contains(NFANodesInvolved)) {
        m_DFAs[NFANodesInvolved] = std::move(dfaNode);
    }
    // else {
    //     assert(m_DFAs[NFANodesInvolved] == dfaNode);
    // }
    return getDFANode(NFANodesInvolved);
}

DFANode* NodeManager::getDFANode(const NFASet& NFAs) {
    return &(m_DFAs.at(NFAs));
}

void NodeManager::makeDFATransitions(DFANode* dfaNode) {
    for (auto const* nfaNode : dfaNode->m_NFANodeSet) {
        for (const auto& [sym, tos] : nfaNode->transitions) {
            if (sym != EPS and not dfaNode->hasTransition(sym)) {
                DFANode* nextDfaNode = makeDFANode(tos);
                dfaNode->addTransition(sym, nextDfaNode);
                makeDFATransitions(nextDfaNode);
            }
        }
    }
}

ReParser::ReParser(std::string_view re) {
    NFANode* nfa = m_nodeManager.NFAFromRe(re);
    m_dfa = m_nodeManager.DFAFromNFA(nfa);
}

bool ReParser::matchExact(std::string_view str) const {
    assert(m_dfa != nullptr);
    return m_dfa->accept(str);
}

int32_t ReParser::find(std::string_view str) const {
    return -1;
}

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
