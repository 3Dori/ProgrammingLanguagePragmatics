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

private:
    DFANode* DFAFromNFA(NFANode*);  // TODO simplify DFA
    NFANode* NFAFromRe(std::string_view);

    NFANode* makeNFANode(const bool isFinal = false);
    NFA makeSymol(const char sym);
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
    /**
     * Use STL containers to automatically manage resourses
     * and remove the need to use smart pointers, which could
     * produce circular references.
     * 
     * Be wary of container resizing, at which time the Nodes
     * are copied and the pointers to the nodes (in transitions)
     * are invalidated, resulting in undefined behaviors.
     */
    std::list<NFANode> m_NFAs;          // unlike vector, lists are never resized
    std::map<NFASet, DFANode> m_DFAs;   // unlike unordered_map, maps are never resized
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
