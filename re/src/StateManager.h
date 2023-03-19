#pragma once

#include "FA.h"
#include "REDef.h"

#include <vector>
#include <list>
#include <map>
#include <set>

namespace RE {

class REParsingStack;

/**
 * Manages the life cycle of NFA and DFA states
 */
class StateManager {
    friend class REParserImpl;
    friend class DFAMinimizer;

private:
    // NFA
    NFA concatenateNFAs(std::vector<NFA>&);

    NFAState* makeNFAState(const bool isFinal = false);

    NFA makeSymbol(const char);
    NFA makeConcatenation(NFA&, NFA&);
    NFA makeAlternation(NFA&, NFA&);
    NFA makeAlternation(std::vector<NFA>&);
    NFA makeCharset(std::string_view);
    NFA makeDigit() {
        return makeCharset("0123456789");
    }

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

        bool containsNfaState(NFAState const* nfaState) const {
            return nfasInvolved.find(nfaState) != nfasInvolved.end();
        }

        void addNfaState(NFAState const* nfaState) {
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
};

} // namespace RE
