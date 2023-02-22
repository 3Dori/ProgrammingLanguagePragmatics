#include "re.h"

#include <iostream>

namespace Re {

uint32_t NFANode::findRightParen(const char* re, uint32_t start) {
    auto numParenthesis = 0u;
    // TODO performance? One pass
    auto end = start + 1;
    while (re[end]) {
        switch (re[end]) {
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

NFANodeSP NFANode::constructNFA(const char* re, uint32_t start, uint32_t end) {
    NFANode* startNode = new NFANode();
    while (start < end and re[start]) {
        const auto sym = re[start];
        switch (sym) {
            case LEFT_PAREN: {
                auto rightParenIdx = findRightParen(re, start);
                auto subNFA = constructNFA(re, start + 1, rightParenIdx - 1);
                startNode->addTransition(EPS, subNFA);
                start = rightParenIdx;
                break;
            }
            case RIGHT_PAREN:
                throw ParenthesisMatchingException(
                    "Unexpected right parenthesis: " + std::string(re, start, end));
                break;
            case BAR:
                break;
            case KLEENE_STAR:
                break;
            default: {
                auto subNFA = constructNFA(re, start + 1, end);
                startNode->addTransition(sym, subNFA);
            }
        }
        start++;
    }
    return NFANodeSP{startNode};
}

}

int main(void) {
    auto r1 = Re::NFANode::constructNFA("abcd");
    auto r2 = Re::NFANode::constructNFA("()");
    auto r3 = Re::NFANode::constructNFA("aa((aaaa)");
    
    return 0;
}
