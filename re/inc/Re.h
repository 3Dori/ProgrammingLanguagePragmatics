#include "ReUtility.h"
#include "FA.h"

#include <string_view>
#include <vector>
#include <list>
#include <map>
#include <cstdint>

namespace Re
{

/**
 * Manages the life cycle of NFA and DFA nodes
 */
struct NodeManager {
    struct NFA {
        NFANode* startNode = nullptr;
        NFANode* endNode = nullptr;
        enum class Type {
            symbol,
            concatenation,
            kleene_star,
            alternation,
            unimplemented
        } type;
    };

    DFANode* DFAFromNFA(NFANode*);
    NFANode* NFAFromRe(std::string_view);

private:
    NFANode* makeNFANode(const bool isFinal = false);
    NFA makeSym(const char sym);
    NFA makeConcatenation(NFA& a, NFA& b);
    NFA makeAlternation(std::vector<NFA>& nodes);
    NFA makeKleeneClousure(NFA& nfa);

    DFANode* makeDFANode(const std::vector<NFANode const*>& nfaNodes);

    inline DFANode* makeDFANode(NFANode const* nfaNode) {
        return makeDFANode(std::vector<NFANode const*>{nfaNode});
    }

    DFANode* getDFANode(const NFASet& NFAs);
    void makeDFATransitions(DFANode* dfaNode);

private:
    std::list<NFANode> m_NFAs;          // never resized compared to vector
    std::map<NFASet, DFANode> m_DFAs;   // never resized compared to unordered_map
};


class ReParser {
public:
    ReParser(std::string_view);
    bool matchExact(std::string_view) const;
    int32_t find(std::string_view) const;

private:
    NodeManager m_nodeManager;
    DFANode* m_dfa = nullptr;
};

} // namespace Re
