#include "DFAMinimizer.h"
#include "StateManager.h"

#include <algorithm>

namespace RE {

DFAMinimizer::DFAMinimizer(StateManager& stateManager) {
    addDeadState(stateManager);
    mergeFinalAndNonFinalStates(stateManager);
}

void DFAMinimizer::mergeFinalAndNonFinalStates(const StateManager& stateManager) {
    m_DFAToMergedDFA.resize(stateManager.m_DFAs.size(), -1);

    constexpr auto NON_FINALS = 0u;
    constexpr auto FINALS = 1u;
    makeMergedDfaState(NON_FINALS);
    makeMergedDfaState(FINALS);
    auto& nonFinals = m_mergedDfaStates.at(NON_FINALS);
    auto& finals = m_mergedDfaStates.at(FINALS);
    for (const auto& [_, dfaState] : stateManager.m_DFAs) {
        const auto id = dfaState.m_id;
        if (dfaState.m_isFinal) {
            finals.dfaStates.insert(&dfaState);
            m_DFAToMergedDFA[id] = FINALS;
        }
        else {
            nonFinals.dfaStates.insert(&dfaState);
            m_DFAToMergedDFA[id] = NON_FINALS;
        }
    }
    if (nonFinals.dfaStates.empty()) {
        m_mergedDfaStates.erase(NON_FINALS);
    }
}

DFA DFAMinimizer::minimize() {
    bool hasAmbiguity;
    do {
        hasAmbiguity = false;
        for (auto& [_, mergedDfaState] : m_mergedDfaStates) {
            if (const char sym = searchForAmbiguousSymbol(mergedDfaState)) {  // TODO memoize ambiguous symbol
                splitMergedDfaState(mergedDfaState, sym);
                hasAmbiguity = true;
                break;
            }
        }
    } while (hasAmbiguity);

    if (m_deadState) {
        removeDeadState();
    }
    return constructMinimizedDFA();
}

DFAMinimizer::MergedDfaState* DFAMinimizer::makeMergedDfaState(const bool isFinal) {
    const auto id = m_mergedDfaStateId;
    m_mergedDfaStates.try_emplace(id, id, isFinal);  // m_mergedDfaStates[id] = MergedDfaState(id, isFinal);
    m_mergedDfaStateId++;
    return &(m_mergedDfaStates.at(id));
}

void DFAMinimizer::splitMergedDfaState(const MergedDfaState& state, const char sym) {
    std::map<int32_t, MergedDfaState*> newTransitions;
    for (auto const* dfaState : state.dfaStates) {
        auto const* toState = dfaState->m_transitions.at(sym);
        int32_t to = m_DFAToMergedDFA[toState->m_id];

        if (newTransitions.find(to) == newTransitions.end()) {
            newTransitions[to] = makeMergedDfaState(state.isFinal);
        }
        newTransitions[to]->dfaStates.insert(dfaState);
        m_DFAToMergedDFA[dfaState->m_id] = newTransitions[to]->id;
    }
    
    m_mergedDfaStates.erase(state.id);
}

char DFAMinimizer::searchForAmbiguousSymbol(const MergedDfaState& mergedDfa) const {
    std::map<char, size_t> transitions;
    for (auto const* dfa : mergedDfa.dfaStates) {
        for (auto [sym, to] : dfa->m_transitions) {
            const auto mergedDfaStateTo = m_DFAToMergedDFA[to->m_id];
            if (transitions.find(sym) == transitions.end()) {
                transitions[sym] = mergedDfaStateTo;
            }
            else if (transitions.at(sym) != mergedDfaStateTo) {  // has ambiguity
                return sym;
            }
        }
    }
    return 0;
}

DFA DFAMinimizer::constructMinimizedDFA() const {
    DFA minimizedDFA;
    for (const auto& [id, mergedDfaState] : m_mergedDfaStates) {
        minimizedDFA.m_states.try_emplace(
            id,
            id, mergedDfaState.isFinal);
    }

    for (const auto& [_, mergedDfaState] : m_mergedDfaStates) {
        mergeTransitions(mergedDfaState, minimizedDFA);
    }
    const int32_t start = m_DFAToMergedDFA[0];
    minimizedDFA.setStart(start);
    return minimizedDFA;
}

void DFAMinimizer::mergeTransitions(const MergedDfaState& from, DFA& dfa) const {
    auto& minimizedState = dfa.m_states.at(from.id);
    for (auto const* dfaState : from.dfaStates) {
        for (const auto& [sym, to] : dfaState->m_transitions) {
            if (not minimizedState.hasTransition(sym) and to != m_deadState) {
                auto const mergedTo = m_DFAToMergedDFA[to->m_id];
                minimizedState.addTransition(sym, &(dfa.m_states.at(mergedTo)));
            }
        }
    }
}

void DFAMinimizer::addDeadState(StateManager& stateManager) {
    auto& dfaStates = stateManager.m_DFAs;
    const auto& inputSymbols = stateManager.m_inputSymbols;

    const auto anyDfaStateHasNoTransition = [&dfaStates](const char sym) {
        for (auto const& [_, dfaState] : dfaStates) {
            if (not dfaState.hasTransition(sym)) {
                return true;
            }
        }
        return false;
    };
    
    bool needDeadState = std::any_of(
        inputSymbols.begin(),
        inputSymbols.end(),
        anyDfaStateHasNoTransition
    );
    if (not needDeadState) {
        return;
    }

    const auto& [keyValue, _] = dfaStates.try_emplace(
        NFAStateSet(),  // Use empty set as a placeholder for key
        stateManager.m_DFAs.size(),
        false);
    m_deadState = &(keyValue->second);
    for (auto& [_, dfaState] : dfaStates) {
        for (auto const sym : inputSymbols) {
            if (not dfaState.hasTransition(sym)) {
                dfaState.addTransition(sym, m_deadState);
            }
        }
    }
}

void DFAMinimizer::removeDeadState() {
    const auto deadState = m_DFAToMergedDFA.back();
    assert(m_mergedDfaStates.at(deadState).dfaStates.size() == 1);
    m_mergedDfaStates.erase(deadState);
}

} // namespace RE
