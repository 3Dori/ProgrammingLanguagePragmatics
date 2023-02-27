#include "re.h"

#include <iostream>
#include <vector>
#include <memory>

namespace Re {

constexpr char EPS = 0;
constexpr char LEFT_PAREN = '(';
constexpr char RIGHT_PAREN = ')';
constexpr char BAR = '|';
constexpr char KLEENE_STAR = '*';

struct NFA {
    NFANodeSP startNode;
    NFANodeSP endNode;
};

struct RePre;
using RePreSP = std::unique_ptr<RePre>;

struct RePre {
    enum class Type {
        concatenation,
        alternation,
        kleene_closure
    } type;
    uint32_t start;
    uint32_t end;
    std::vector<RePreSP> pres;

    RePre(const Type type, const uint32_t start, const uint32_t end)
        : type(type), start(start), end(end)
    {}

    static RePreSP makeRePre(const Type type, const uint32_t start, const uint32_t end) {
        return std::make_unique<RePre>(type, start, end);
    }
};

template <typename AutomataNode>
std::shared_ptr<AutomataNode> makeNode(bool isFinal = false) {
    return std::make_shared<AutomataNode>(isFinal);
}

NFA makeSym(const char sym) {
    auto startNode = makeNode<NFANode>();
    auto endNode = makeNode<NFANode>(true);
    startNode->addTransition(sym, endNode);
    return {startNode, endNode};
}

NFA makeConcatenation(NFA& a, NFA& b) {
    // TODO copy all b.startNode's transitions to a.endNode
    a.endNode->isFinal = false;
    a.endNode->addTransition(EPS, b.startNode);
    return {a.startNode, b.endNode};
}

NFA makeAlternation(std::vector<NFA>& nodes) {
    auto startNode = makeNode<NFANode>();
    auto endNode = makeNode<NFANode>(true);

    for (auto& node : nodes) {
        node.endNode->isFinal = false;
        startNode->addTransition(EPS, node.startNode);
        node.endNode->addTransition(EPS, endNode);
    }

    return {startNode, endNode};
}

NFA makeKleeneClousure(NFA& nfa) {
    auto startNode = makeNode<NFANode>();
    auto endNode = makeNode<NFANode>(true);

    nfa.endNode->isFinal = false;

    startNode->addTransition(EPS, nfa.startNode);
    startNode->addTransition(EPS, endNode);
    nfa.endNode->addTransition(EPS, nfa.startNode);
    nfa.endNode->addTransition(EPS, endNode);

    return {startNode, endNode};
}

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
            auto rePre = RePre::makeRePre(RePre::Type::concatenation, start+1, end-1);
            if (re[end+1] == KLEENE_STAR) {
                rePre->type = RePre::Type::kleene_closure;
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

void DFANode::addTransition(const char sym, const NFANodeSP& to) {
    if (not hasTransition(sym)) {
        transitions[sym] = DFANodeSP{new DFANode(false)};
    }
    transitions[sym]->NFANodes.insert(to);
    transitions[sym]->bypassEPS(to);
}

// TODO possible infinite loop? FIXED by checking containment
void DFANode::bypassEPS(const NFANodeSP& nfa) {
    for (const auto& transition : nfa->transitions) {
        if (transition.sym == EPS and
            not hasState(transition.to))
        {
            NFANodes.insert(transition.to);
            bypassEPS(transition.to);  // in a DFS manner
        }
    }
}

void DFANode::bfsNFAToDFA() {
    for (const auto& nfaNode : NFANodes) {
        // TODO check duplicates of states
        for (const auto& [sym, to] : nfaNode->transitions) {
            if (sym == EPS) {
                continue;
            }
            if (hasTransition(sym) and transitions[sym]->hasState(to)) {
                continue;
            }
            else {
                addTransition(sym, to);
            }
        }
    }
    // TODO infinite loop when a node has a transition to itself
    for (auto& [_, to] : transitions) {
        to->bfsNFAToDFA();
    }
}

DFANodeSP DFANode::NFAToDFA(const NFANodeSP& nfa) {
    DFANodeSP start = makeNode<DFANode>();
    start->NFANodes.insert(nfa);
    start->bypassEPS(nfa);
    start->bfsNFAToDFA();
    return start;
}

bool DFANode::hasState(const NFANodeSP& nfa) const {
    return NFANodes.contains(nfa);
}

bool DFANode::hasTransition(const char sym) const {
    return transitions.contains(sym);
}

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
