#include "ReUtility.h"
#include "FA.h"

#include <string_view>
#include <vector>
#include <unordered_map>

namespace Re
{

/**
 * Manages the life cycle of NFA and DFA nodes
 */
struct NodeManager {
    struct NFA {
        NFANode* startNode = nullptr;
        NFANode* endNode = nullptr;
    };

    DFANode* DFAFromNFA(NFANode*);
    NFANode* NFAFromRe(std::string_view);

private:
    NFANode* makeNFANode(const bool isFinal = false);
    NFA makeSym(const char sym);
    NFA makeConcatenation(NFA& a, NFA& b);
    NFA makeAlternation(std::vector<NFA>& nodes);
    NFA makeKleeneClousure(NFA& nfa);

    DFANode* makeDFANode(const std::vector<NFANode*>& nfaNodes);

    inline DFANode* makeDFANode(NFANode* nfaNode) {
        return makeDFANode({nfaNode});
    }

    DFANode* getDFANode(const NFASet& NFAs);
    void makeDFATransitions(DFANode* dfaNode);

private:
    std::vector<NFANode> m_NFAs;
    std::unordered_map<NFASet, DFANode> m_DFAs;
};


class ReParser {
public:
    ReParser(std::string_view);
    bool match(std::string_view);

private:
    NodeManager m_nodeManager;
};

} // namespace Re
