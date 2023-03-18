#include "REDef.h"
#include "FA.h"
#include "StateManager.h"

#include <REExceptions.h>

#include <cassert>

namespace RE
{

// NFA

NFA StateManager::concatenateNFAs(std::vector<NFA>& nfas) {
    // TODO investigate it
    // NFA emptyNfa;
    // NFA nfa = std::accumulate(nfas.begin(), nfas.end(), emptyNfa,
                            //   std::bind(&StateManager::makeConcatenation, this, _1, _2));
                            //   [this](NFA& a, NFA& b) {
                            //       return makeConcatenation(a, b);
                            //   });
    NFA resultNfa;
    for (auto& nfa : nfas) {
        resultNfa = makeConcatenation(resultNfa, nfa);
    }
    return { resultNfa.startState, resultNfa.endState };
}

NFAState* StateManager::makeNFAState(const bool isFinal) {
    m_NFAs.emplace_back(m_NFAs.size(), isFinal);
    assert(m_NFAs.back().m_id == m_NFAs.size() - 1 and "Wrong NFAState id");
    return &(m_NFAs.back());
}

NFA StateManager::makeSymbol(const char sym) {
    auto startState = makeNFAState();
    auto endState = makeNFAState(true);
    startState->addTransition(sym, endState);
    m_inputSymbols.insert(sym);
    return { startState, endState };
}

NFA StateManager::makeConcatenation(NFA& a, NFA& b) {
    if (a.isEmpty()) {
        return b;
    }
    if (b.isEmpty()) {
        return a;
    }
    a.endState->m_isFinal = false;
    a.endState->addTransition(EPS, b.startState);
    return { a.startState, b.endState };
}

NFA StateManager::makeAlternation(NFA& a, NFA& b) {
    if (a.isEmpty() and b.isEmpty()) {
        return a;
    }
    else if (a.isEmpty()) {
        return makeQuestion(b);
    }
    else if (b.isEmpty()) {
        return makeQuestion(a);
    }

    auto startState = makeNFAState();
    auto endState = makeNFAState(true);

    a.endState->m_isFinal = false;
    b.endState->m_isFinal = false;

    startState->addTransition(EPS, a.startState);
    startState->addTransition(EPS, b.startState);

    a.endState->addTransition(EPS, endState);
    b.endState->addTransition(EPS, endState);

    return {startState, endState};
}

NFA StateManager::makeKleeneClousure(NFA& nfa) {
    nfa.startState->addTransition(EPS, nfa.endState);
    nfa.endState->addTransition(EPS, nfa.startState);
    return { nfa.startState, nfa.endState };
}

NFA StateManager::makePlus(NFA& nfa) {
    nfa.endState->addTransition(EPS, nfa.startState);
    return { nfa.startState, nfa.endState };
}

NFA StateManager::makeQuestion(NFA& nfa) {
    nfa.startState->addTransition(EPS, nfa.endState);
    return { nfa.startState, nfa.endState };
}

NFA StateManager::makeCopy(const NFA& nfa) {
    if (nfa.isEmpty()) {
        return nfa;
    }
    std::map<NFAState const*, NFAState*> copied;
    copied[nfa.startState] = makeNFAState();
    copyTransitions(nfa.startState, copied);
    return { copied.at(nfa.startState), copied.at(nfa.endState) };
}

void StateManager::copyTransitions(NFAState const* copyFrom, std::map<NFAState const*, NFAState*>& copied) {
    for (const auto& [sym, tos] : copyFrom->m_transitions) {
        for (auto const* to : tos) {
            if (copied.find(to) == copied.end()) {
                copied[to] = makeNFAState(to->m_isFinal);
                copyTransitions(to, copied);
            }
            copied.at(copyFrom)->addTransition(sym, copied.at(to));
        }
    }
}

// DFA

DFAStateFromNFA* StateManager::DFAFromNFA(NFAState const* nfa) {
    const auto dfaInfo = mergeEPSTransitions(nfa);
    DFAStateFromNFA* dfa = getDFAState(dfaInfo);
    generateDFATransitions(dfa);
    return dfa;
}

DFAStateFromNFA* StateManager::getDFAState(const DFAInfo& dfaInfo) {
    const auto nfasInvolved = dfaInfo.nfasInvolved;
    if (m_DFAs.find(dfaInfo.nfasInvolved) == m_DFAs.end()) {
        m_DFAs.try_emplace(
            dfaInfo.nfasInvolved,
            m_DFAs.size(),
            dfaInfo.isFinal,
            dfaInfo.nfasInvolved);
    }
    return &(m_DFAs.at(nfasInvolved));
}

void StateManager::generateDFATransitions(DFAStateFromNFA* dfaState) {
    for (const auto sym : m_inputSymbols) {
        assert(sym != EPS);
        if (dfaState->hasTransition(sym)) {
            continue;
        }
        const auto dfaInfo = mergeTransitions(dfaState, sym);
        DFAStateFromNFA* to = getDFAState(dfaInfo);
        dfaState->addTransition(sym, to);
        generateDFATransitions(to);
    }
}

StateManager::DFAInfo StateManager::mergeEPSTransitions(NFAState const* nfaState) {
    DFAInfo dfaInfo;
    mergeEPSTransitions(nfaState, dfaInfo);
    return dfaInfo;
}

void StateManager::mergeEPSTransitions(NFAState const* nfaState, DFAInfo& dfaInfo) {
    if (dfaInfo.containsNfaState(nfaState)) {
        return;
    }
    dfaInfo.addNfaState(nfaState);
    if (nfaState->m_isFinal) {
        dfaInfo.isFinal = true;
    }
    if (not nfaState->hasTransition(EPS)) {
        return;
    }
    for (auto const* to : nfaState->m_transitions.at(EPS)) {
        mergeEPSTransitions(to, dfaInfo);
    }
}

StateManager::DFAInfo StateManager::mergeTransitions(DFAStateFromNFA const* dfaState, const char sym) {
    DFAInfo dfaInfo;
    for (auto const* nfaState : dfaState->m_NFAStateSet) {
        if (nfaState->hasTransition(sym)) {
            for (auto const* to : nfaState->m_transitions.at(sym)) {
                mergeEPSTransitions(to, dfaInfo);
            }
        }
    }
    return dfaInfo;
}

} // namespace RE
