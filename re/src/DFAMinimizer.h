#pragma once

#include "FA.h"

#include <cstddef>
#include <list>
#include <set>
#include <memory>
#include <vector>

namespace RE
{

class StateManager;

class DFAMinimizer {
public:
    struct MergedDfaState {
        MergedDfaState(const int32_t id, const bool isFinal)
            : id(id), isFinal(isFinal) {}
        int32_t id;
        bool isFinal;
        std::set<DFAStateFromNFA const*> dfaStates;
    };

public:
    DFAMinimizer(std::unique_ptr<StateManager>&);
    std::unique_ptr<DFA> minimize();

private:
    MergedDfaState* makeMergedDfaState(const bool);
    void splitMergedDfaState(const MergedDfaState&, const char);
    char searchForAmbiguousSymbol(const MergedDfaState&) const;
    std::unique_ptr<DFA> constructMinimizedDFA() const;
    void mergeTransitions(const MergedDfaState&, DFA&) const;

    void addDeadState(
        std::vector<DFAStateFromNFA*>&,
        std::set<char>&); /* so that each state has an transition
                             for each input */
    void removeDeadState();
private:
    std::vector<int32_t> m_DFAToMergedDFA;

    std::map<int32_t, MergedDfaState> m_mergedDfaStates;  // std::map has fixed capacity
    int32_t m_mergedDfaStateId = 0;

    DFAStateFromNFA m_deadStateDFA;
    DFAStateFromNFA* m_deadState = nullptr;
};

} // namespace RE
