#include "FA.h"

namespace Re {

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

}  // namespace Re
