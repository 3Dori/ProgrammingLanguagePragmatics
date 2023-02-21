#include "re.h"

#include <iostream>

namespace Re {

uint32_t NFANode::findRightParen(const std::string& re, uint32_t start) {
    auto numParenthesis = 0u;
    // TODO performance? One pass
    for (auto end = start + 1; end < re.size(); end++) {
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
    }
    throw ParenthesisMatchingException("Cannot match right parenthesis: " + re.substr(start));
}

NFANodeSP NFANode::constructNFA(const std::string& re) {
    NFANode* startNode = new NFANode();
    for (auto i = 0; i < re.size(); i++) {
        switch (re[i]) {
            case LEFT_PAREN: {
                auto rightParenIdx = findRightParen(re, i);
                auto subNFA = constructNFA(re.substr(i + 1, rightParenIdx - 1));
                startNode->addTransition(EPS, subNFA);
                i = rightParenIdx;
                break;
            }
            case RIGHT_PAREN:
                throw ParenthesisMatchingException("Unexpected right parenthesis: " + re.substr(0, i + 1));
                break;
            case BAR:
                break;
            case KLEENE_STAR:
                break;
            default:
                break;
        }
    }
    return NFANodeSP{startNode};
}

}

int main(void) {
    auto r1 = Re::NFANode::constructNFA("abcd");
    auto r2 = Re::NFANode::constructNFA("()");
    auto r3 = Re::NFANode::constructNFA("(())");
    
    return 0;
}
