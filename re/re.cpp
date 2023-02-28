#include "NodeManager.h"
#include "re.h"

#include <iostream>
#include <vector>
#include <memory>

namespace Re {

struct ReParser;
using RePreSP = std::unique_ptr<ReParser>;

struct ReParser {
    enum class Type {
        concatenation,
        alternation,
        kleene_closure
    } type;
    uint32_t start;
    uint32_t end;
    std::vector<RePreSP> pres;

    ReParser(const Type type, const uint32_t start, const uint32_t end)
        : type(type), start(start), end(end)
    {}

    static RePreSP makeRePre(const Type type, const uint32_t start, const uint32_t end) {
        return std::make_unique<ReParser>(type, start, end);
    }
};

int32_t findRightParen(const char* re, uint32_t start) {
    auto numParenthesis = 0u;
    // TODO performance? One pass
    auto end = start + 1;
    while (re[end]) {
        switch (re[end])
        {
        case LEFT_PAREN:
            numParenthesis++;
            break;
        case RIGHT_PAREN:
            if (numParenthesis == 0) {
                return end;
            }
            else {                                
                numParenthesis--;
            }
            break;
        default:
            break;
        }
        end++;
    }
    throw ParenthesisMatchingException(
        "Missing right parenthesis: " + std::string(re + start));
}

RePreSP parseReToPre(const char* re, uint32_t start, uint32_t end) {
    while (start < end and re[start]) {
        const auto sym = re[start];
        switch (sym)
        {
        case LEFT_PAREN: {
            auto end = findRightParen(re, start);
            auto rePre = ReParser::makeRePre(ReParser::Type::concatenation, start+1, end-1);
            if (re[end+1] == KLEENE_STAR) {
                rePre->type = ReParser::Type::kleene_closure;
            }
            start = end;
        }
        case RIGHT_PAREN:
            throw ParenthesisMatchingException(
                "Unexpected right parenthesis: " + std::string(re, start, end));
            break;
        }
        start++;
    }
}

void DFANode::bypassEPS(NFANode* nfaNode) {
    if (nfaNode->m_isFinal) {
        m_isFinal = true;
    }
    m_NFANodes[nfaNode->m_id] = true;
    m_NFANodeSet.insert(nfaNode);

    const auto& tos = nfaNode->transitions[EPS];
    for (auto* to : tos) {  // in a DFS manner
        if (not m_NFANodes[to->m_id]) {
            bypassEPS(to);
        }
    }
}

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

}

int main(void) {
    // auto r1 = Re::NFANode::constructNFA("abcd");
    // auto r2 = Re::NFANode::constructNFA("()");
    // auto r3 = Re::NFANode::constructNFA("aa((aaaa)");
    
    return 0;
}
