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
 * Manages the life cycle of NFA and DFA states
 */
class StateManager {
    friend class REParser;
    friend class DFAMinimizer;

public:
    struct NFA {
        NFAState* startState = nullptr;
        NFAState* endState = nullptr;

        bool isEmpty() const {
            return startState == nullptr;
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
    NFAState* NFAFromRe(std::string_view);
    /**
     * Pop the parsing stack and push the NFA representing a group until:
     * switch (type)
     *   GroupStartType::parenthesis:  the last open parenthsis
     *   GroupStartType::re_start:     the bottom of the stack
     *   GroupStartType::bar:          the last open parenthsis or the bottom of the stack
    */
    NFA parseLastGroup(REParsingStack&, const size_t, const GroupStartType);

    NFA concatenateNFAs(std::vector<NFA>&);

    NFAState* makeNFAState(const bool isFinal = false);

    NFA makeSymbol(const char);
    NFA makeConcatenation(NFA&, NFA&);
    NFA makeAlternation(NFA&, NFA&);

    NFA checkRepetitionAndPopLastNfa(REParsingStack&, const size_t, const bool);
    NFA makeKleeneClousure(NFA&);
    NFA makePlus(NFA&);
    NFA makeQuestion(NFA&);
    NFA makeCopy(const NFA&);
    void copyTransitions(NFAState const*, std::map<NFAState const*, NFAState*>&);

    // DFA
    DFAStateFromNFA* DFAFromNFA(NFAState const*);

    struct DFAInfo {
        NFAStateSet nfasInvolved;
        bool isFinal = false;

        inline bool containsNfaState(NFAState const* nfaState) const {
            return nfasInvolved.find(nfaState) != nfasInvolved.end();
        }

        inline void addNfaState(NFAState const* nfaState) {
            nfasInvolved.insert(nfaState);
        }
    };

    DFAStateFromNFA* getDFAState(const DFAInfo&);
    void generateDFATransitions(DFAStateFromNFA*);
    static DFAInfo mergeEPSTransitions(NFAState const*);
    static void mergeEPSTransitions(NFAState const*, DFAInfo&);
    static DFAInfo mergeTransitions(DFAStateFromNFA const*, const char);

private:
    std::set<char> m_inputSymbols;
    /**
     * Use STL containers to automatically manage resourses and remove the
     * need to use smart pointers, which could produce circular references.
     *
     * Be wary of container changing its capacity, at which time the state
     * are copied and the pointers to the state (in transitions) are
     * invalidated, resulting in undefined behaviors.
     */
    /* unlike vector, lists don't change their capacity */
    std::list<NFAState> m_NFAs;
    /* unlike unordered_map, maps don't change their capacity */
    std::map<NFAStateSet, DFAStateFromNFA> m_DFAs;

    std::vector<DFAStateFromNFA*> m_DFAsIndexed;  // TODO refactor out this, the interface to call minimize is weird
};

} // namespace RE
