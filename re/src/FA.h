#pragma once

#include "REDef.h"

#include <cassert>
#include <vector>
#include <string_view>
#include <set>
#include <map>

namespace RE {

// TODO rename to states?
class NFAState {
    friend class REParser;
    friend class StateManager;
    friend class DFAStateFromNFA;

public:
    NFAState(const size_t id, const bool isFinal) :
        m_id(id), m_isFinal(isFinal)
    {}

private:
    NFAState(const NFAState&) = delete;
    NFAState& operator=(const NFAState&) = delete;

    void addTransition(const char, NFAState const*);
    bool hasTransition(const char) const;

    const size_t m_id;
    bool m_isFinal;
    std::map<char, NFAStateSet> m_transitions;
};

class DFAState {
    friend class DFAMinimizer;

public:
    DFAState(const size_t id, const bool isFinal = false)
        : m_id(id), m_isFinal(isFinal) {}

    bool accept(std::string_view) const;

private:
    DFAState(const DFAState&) = delete;
    DFAState& operator=(const DFAState&) = delete;

protected:
    void addTransition(const char, DFAState const*);
    bool hasTransition(const char) const;

protected:
    size_t m_id;  // TODO: eliminate the need to use id
    bool m_isFinal = false;
    std::map<char, DFAState const*> m_transitions;
};

class DFAStateFromNFA : public DFAState {
    friend class REParser;
    friend class StateManager;
    friend class NFAState;
    friend class DFAMinimizer;

public:
    DFAStateFromNFA(const size_t id, const bool isFinal = false)
        : DFAState(id, isFinal) {}
    DFAStateFromNFA(const size_t id, const bool isFinal, const NFAStateSet& nfas)
        : DFAState(id, isFinal), m_NFAStateSet(nfas) {}

   private:
    bool hasState(NFAState const*) const;

private:
    NFAStateSet m_NFAStateSet;
};


class DFA {    
    friend class DFAMinimizer;

public:
    inline bool accept(std::string_view str) const {
        return m_start->accept(str);
    }

private:
    void setStart(const int32_t start) {
        m_start = &(m_states.at(start));
    }

private:
    std::map<int32_t, DFAState> m_states;  // actual storage
    DFAState const* m_start;
};

} // namespace RE
