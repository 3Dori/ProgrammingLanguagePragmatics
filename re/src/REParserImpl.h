#pragma once

#include "FA.h"
#include "REParsingStack.h"
#include "StateManager.h"

namespace RE
{

class REParserImpl {
public:
    REParserImpl(std::string_view re);
    bool matchExact(std::string_view str) const {
        return m_dfa.accept(str);
    }

private:
    NFAState* NFAFromRe(std::string_view);

private:
    /**
     * Pop the parsing stack and push the NFA representing a group until:
     * switch (type)
     *   GroupStartType::parenthesis:  the last open parenthsis
     *   GroupStartType::re_start:     the bottom of the stack
     *   GroupStartType::bar:          the last open parenthsis or the bottom of
     * the stack
     */
    NFA parseLastGroup(const size_t, const REParsingStack::GroupStartType);

private:
    StateManager m_stateManager;
    REParsingStack m_stack;
    DFA m_dfa;
};

} // namespace RE
