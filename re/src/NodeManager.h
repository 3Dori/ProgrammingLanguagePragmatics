#pragma once

#include "FA.h"
#include "REDef.h"

#include <vector>
#include <list>
#include <map>
#include <memory>
#include <set>

namespace RE {

class REParsingStack;

/**
 * Manages the life cycle of NFA and DFA nodes
 */
class NodeManager {
    friend class REParser;

public:
    struct NFA {
        NFANode* startNode = nullptr;
        NFANode* endNode = nullptr;
        enum class Type {
            symbol,
            concatenation,
            repeat,
            alternation,
            unimplemented
        } type;

        bool isEmpty() const {
            return startNode == nullptr;
        }
    };

    enum class GroupStartType {
        bar = 0u,
        parenthesis = 1u,
        re_start = 2u,
    };

    /**
     * predecence: re_start > parenthesis > bar
    */
    inline static bool hasHigherOrEqualPredecence(const GroupStartType a, const GroupStartType b) {
        return static_cast<uint32_t>(a) >= static_cast<uint32_t>(b);
    }

private:
    // NFA
    NFANode* NFAFromRe(std::string_view);
    /**
     * Pop the parsing stack and push the NFA representing a group until:
     * switch (type)
     *   GroupStartType::parenthesis:  the last open parenthsis
     *   GroupStartType::re_start:     the bottom of the stack
     *   GroupStartType::bar:          the last open parenthsis or the bottom of the stack
    */
    NFA parseLastGroup(REParsingStack&, const size_t, const GroupStartType);

    NFA concatenateNFAs(std::vector<NFA>&);

    NFANode* makeNFANode(const bool isFinal = false);
    NFA makeSymbol(const char);
    NFA makeConcatenation(NFA&, NFA&);
    NFA makeAlternation(NFA&, NFA&);

    NFA makeKleeneClousure(NFA&);
    NFA makePlus(NFA&);
    NFA makeQuestion(NFA&);

    // DFA
    DFANodeFromNFA* DFAFromNFA(NFANode*);
    inline DFANodeFromNFA makeDFANode() {
        return DFANodeFromNFA(m_DFAs.size());
    }
    DFANodeFromNFA* getDFANode(const std::set<NFANode const*>&);
    inline DFANodeFromNFA* getDFANode(NFANode const* nfaNode) {
        return getDFANode(std::set<NFANode const*>{nfaNode});
    }

    DFANodeFromNFA* tryAddAndGetDFANode(DFANodeFromNFA&);
    void generateDFATransitions(DFANodeFromNFA*);

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
    std::map<NodeSet, DFANodeFromNFA> m_DFAs;   // unlike unordered_map, maps are never resized

    std::vector<DFANodeFromNFA*> m_DFAsIndexed;  // TODO refactor out this, the interface to call minimize is weird
};

} // namespace RE
