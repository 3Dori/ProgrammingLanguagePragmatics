#include "RE.h"

#include <iostream>
#include <numeric>
#include <string_view>
#include <functional>
#include <vector>

// using std::placeholders::_1;
// using std::placeholders::_2;  TODO remove

namespace RE {

// NFA
NFANode* NodeManager::NFAFromRe(std::string_view re) {
    assert(m_resultNfa.empty() and "m_resultNfa must be empty");
    for (auto pos = 0u; pos < re.size(); ++pos) {
        const auto sym = re[pos];
        switch (sym)
        {
        case EPS: assert(false and "Unexpected end of regex");
        case BAR:
        case LEFT_PAREN:
        case RIGHT_PAREN:
            assert(false and "Unimplemented");
        case KLEENE_STAR:
        case PLUS:
        case QUESTION:
            m_resultNfa.push_back(makeRepeat(sym, pos));
            break;
        case ESCAPE:
        {
            pos++;  // advance one symbol manually
            if (pos == re.size()) {
                throw EscapeException("Escape reaches the end of the input");
            }
            const auto nextSym = re[pos];
            switch (nextSym) {
                case BAR:
                case LEFT_PAREN:
                case RIGHT_PAREN:
                case KLEENE_STAR:
                case PLUS:
                case QUESTION:
                case ESCAPE:
                    m_resultNfa.push_back(makeSymbol(nextSym));
                    break;
                default:
                    throw EscapeException(nextSym, pos);
            }
            break;
        }
        default:
            m_resultNfa.push_back(makeSymbol(sym));
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
    for (auto& nfa : m_resultNfa) {
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

NodeManager::NFA NodeManager::makeSymbol(const char sym) {
    auto startNode = makeNFANode();
    auto endNode = makeNFANode(true);
    startNode->addTransition(sym, endNode);
    return {startNode, endNode, NFA::Type::symbol};
}

NodeManager::NFA NodeManager::makeConcatenation(NFA& a, NFA& b) {
    if (a.startNode == nullptr) {
        return b;
    }
    a.endNode->m_isFinal = false;
    a.endNode->addTransition(EPS, b.startNode);
    return {a.startNode, b.endNode, NFA::Type::concatenation};
}

NodeManager::NFA NodeManager::makeAlternation(NFA& a, NFA& b) {
    auto startNode = makeNFANode();
    auto endNode = makeNFANode(true);

    a.endNode->m_isFinal = false;
    b.endNode->m_isFinal = false;

    startNode->addTransition(EPS, a.startNode);
    startNode->addTransition(EPS, b.endNode);

    a.endNode->addTransition(EPS, endNode);
    b.endNode->addTransition(EPS, endNode);

    return {startNode, endNode, NFA::Type::alternation};
}

NodeManager::NFA NodeManager::makeKleeneClousure(NFA& nfa) {
    nfa.startNode->addTransition(EPS, nfa.endNode);
    nfa.endNode->addTransition(EPS, nfa.startNode);
    return {nfa.startNode, nfa.endNode, NFA::Type::repeat};
}

NodeManager::NFA NodeManager::makePlus(NFA& nfa) {
    nfa.endNode->addTransition(EPS, nfa.startNode);
    return {nfa.startNode, nfa.endNode, NFA::Type::repeat};
}

NodeManager::NFA NodeManager::makeQuestion(NFA& nfa) {
    nfa.startNode->addTransition(EPS, nfa.endNode);
    return {nfa.startNode, nfa.endNode, NFA::Type::repeat};
}

NodeManager::NFA NodeManager::makeRepeat(const char sym, const size_t pos) {
    if (m_resultNfa.size() == 0) {
        throw NothingToRepeatException(pos);
    }
    NFA lastNfa = m_resultNfa.back();
    if (lastNfa.type == NFA::Type::repeat) {
        throw MultipleRepeatException(pos);
    }
    m_resultNfa.pop_back();
    switch (sym) {
        case KLEENE_STAR:
            return makeKleeneClousure(lastNfa);
        case PLUS:
            return makePlus(lastNfa);
        case QUESTION:
            return makeQuestion(lastNfa);
        default:
            assert(false and "Unexpected repeat symbol");
    }
}

// DFA
DFANode* NodeManager::DFAFromNFA(NFANode* nfa) {
    DFANode* dfa = makeDFANode(nfa);
    makeDFATransitions(dfa);
    return dfa;
}

DFANode* NodeManager::makeDFANode(const std::set<NFANode const*>& nfaNodes) {
    DFANode dfaNode;
    for (auto const* nfaNode : nfaNodes) {
        dfaNode.bypassEPS(nfaNode);
    }

    // create a DFA node if it doesn't exist
    const auto NFANodesInvolved = dfaNode.m_NFANodes;
    if (m_DFAs.find(NFANodesInvolved) == m_DFAs.end()) {
        m_DFAs[NFANodesInvolved] = std::move(dfaNode);
    }
    return getDFANode(NFANodesInvolved);
}

DFANode* NodeManager::getDFANode(const NFASet& NFAs) {
    return &(m_DFAs.at(NFAs));
}

void NodeManager::makeDFATransitions(DFANode* dfaNode) {
    for (auto const* nfaNode : dfaNode->m_NFANodeSet) {
        for (const auto& [sym, tos] : nfaNode->m_transitions) {
            if (sym != EPS and not dfaNode->hasTransition(sym)) {
                DFANode* nextDfaNode = makeDFANode(tos);
                dfaNode->addTransition(sym, nextDfaNode);
                makeDFATransitions(nextDfaNode);
            }
        }
    }
}

REParser::REParser(std::string_view re) {
    NFANode* nfa = m_nodeManager.NFAFromRe(re);
    m_dfa = m_nodeManager.DFAFromNFA(nfa);
}

bool REParser::matchExact(std::string_view str) const {
    assert(m_dfa != nullptr);
    return m_dfa->accept(str);
}

int32_t REParser::find(std::string_view str) const {
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
//             auto rePre = REParser::makeRePre(REParser::Type::concatenation, start+1, end-1);
//             if (re[end+1] == KLEENE_STAR) {
//                 rePre->type = REParser::Type::kleene_closure;
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

} // namespace RE
