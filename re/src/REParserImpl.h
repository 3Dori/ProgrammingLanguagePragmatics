#pragma once

#include "FA.h"
#include "REParsingStack.h"
#include "StateManager.h"

#include <RE.h>

#include <string_view>

namespace RE
{

class REParserImpl {
public:
    REParserImpl(REParser::RE_t re);
    bool matchExact(const std::string_view& str) const {
        return m_dfa.accept(str);
    }

private:
    NFAState* NFAFromRe(REParser::RE_t);

private:
    void advance() noexcept;
    bool checkIsLastStateRepetition(const char) const noexcept;

    REParser::RE_t m_re;
    uint32_t m_pos;
    char m_sym;
    bool m_isLastStateRepetition;

private:
    /**
     * Pop the parsing stack and push the NFA representing a group until:
     * switch (type)
     *   GroupStartType::parenthesis:  the last open parenthsis
     *   GroupStartType::re_start:     the bottom of the stack
     *   GroupStartType::bar:          the last open parenthsis or the bottom of
     * the stack
     */
    NFA makeLastGroup(const REParsingStack::GroupStartType);
    void parseBar() {
        m_stack.push(makeLastGroup(REParsingStack::GroupStartType::bar));
        m_stack.pushBar(m_pos);
    }
    void parseLeftParen() { m_stack.pushOpenParen(m_pos); }
    void parseRightParen() {
        m_stack.push(
            makeLastGroup(REParsingStack::GroupStartType::parenthesis));
    }
    void parseLeftBrace();
    uint32_t parseNumRepetitions(const uint32_t);
    void parseRepetition();
    NFA checkRepetitionAndPopLastNfa();
    void parseEscape();
    void parseSym() { m_stack.push(m_stateManager.makeSymbol(m_sym)); }

private:
    StateManager m_stateManager;
    REParsingStack m_stack;
    DFA m_dfa;
};

} // namespace RE
