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
    friend class DFAMinimizer;

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
        } type;  // remove this field

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

    NFA checkRepetitionAndPopLastNfa(REParsingStack&, const size_t, const bool);

    NFA concatenateNFAs(std::vector<NFA>&);

    NFANode* makeNFANode(const bool isFinal = false);
    NFA makeSymbol(const char);
    NFA makeConcatenation(NFA&, NFA&);
    NFA makeAlternation(NFA&, NFA&);
    NFA makeCopy(const NFA&);
    void copyTransitions(NFANode const*, std::map<NFANode const*, NFANode*>&);

    NFA makeKleeneClousure(NFA&);
    NFA makePlus(NFA&);
    NFA makeQuestion(NFA&);

    // DFA
    DFANodeFromNFA* DFAFromNFA(NFANode const*);

    struct DFAInfo {
        NFANodeSet nfasInvolved;
        bool isFinal = false;

        inline bool containsNfaNode(NFANode const* nfaNode) const {
            return nfasInvolved.find(nfaNode) != nfasInvolved.end();
        }

        inline void addNfaNode(NFANode const* nfaNode) {
            nfasInvolved.insert(nfaNode);
        }
    };

    DFANodeFromNFA* getDFANode(const DFAInfo&);
    void generateDFATransitions(DFANodeFromNFA*);
    static DFAInfo mergeEPSTransitions(NFANode const*);
    static void mergeEPSTransitions(NFANode const*, DFAInfo&);
    static DFAInfo mergeTransitions(DFANodeFromNFA const*, const char);

private:
    std::set<char> m_inputSymbols;
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
    std::map<NFANodeSet, DFANodeFromNFA> m_DFAs;   // unlike unordered_map, maps are never resized

    std::vector<DFANodeFromNFA*> m_DFAsIndexed;  // TODO refactor out this, the interface to call minimize is weird
};

} // namespace RE
