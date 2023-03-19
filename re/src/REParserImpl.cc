#include "DFAMinimizer.h"
#include "REParserImpl.h"
#include "REParsingStack.h"

#include <REExceptions.h>

namespace RE {

REParserImpl::REParserImpl(REParser::RE_t re) :
    m_re(re),
    m_pos(0),
    m_sym(re[0]),
    m_isLastStateRepetition(false)
{
    NFAState* nfa = NFAFromRe(re);
    DFAStateFromNFA* dfa = m_stateManager.DFAFromNFA(nfa);
    m_dfa = DFAMinimizer(m_stateManager).minimize();
}

NFAState* REParserImpl::NFAFromRe(REParser::RE_t re) {
    for (char lastSym = 0;
         m_pos < m_re.size();
         m_isLastStateRepetition = checkIsLastStateRepetition(lastSym), advance(), lastSym = m_sym)
    {
        switch (m_sym) {
        case EPS:
            assert(false and "Unexpected end of regex");
        case BAR:
            parseBar();
            break;
        case LEFT_PAREN:
            parseLeftParen();
            break;
        case RIGHT_PAREN:
            parseRightParen();
            break;
        case LEFT_BRACE:
            parseLeftBrace();
            break;
        case RIGHT_BRACE:
            throw UnbalancedBraceException(m_pos);
        case KLEENE_STAR:
        case PLUS:
        case QUESTION:
            parseRepetition();
            break;
        case ESCAPE:
            parseEscape();
            break;
        default:
            parseSym();
        }
    }
    NFA nfa = makeLastGroup(REParsingStack::GroupStartType::re_start);
    return nfa.isEmpty() ?
           m_stateManager.makeNFAState(true) :
           nfa.startState;
}

void REParserImpl::advance() noexcept {
    m_pos++;
    m_sym = m_re[m_pos];
}

bool REParserImpl::checkIsLastStateRepetition(const char lastSym) const noexcept {
    switch (lastSym) {
        case KLEENE_STAR:
        case PLUS:
        case QUESTION:
        case LEFT_BRACE:
            return true;
        default:
            return false;
    }
}

NFA REParserImpl::makeLastGroup(const REParsingStack::GroupStartType type) {
    while (true) {
        const auto lastGroupStartType = m_stack.getLastGroupStart().type;
        const auto lastGroupStartPosInRe = m_stack.getLastGroupStart().posInRe;
        auto nfas = m_stack.popTillLastGroupStart(type);
        switch (lastGroupStartType) {
        case REParsingStack::GroupStartType::parenthesis:
            switch (type) {
                case REParsingStack::GroupStartType::re_start:
                    throw MissingParenthsisException(lastGroupStartPosInRe);
                case REParsingStack::GroupStartType::bar:
                case REParsingStack::GroupStartType::parenthesis:
                    return m_stateManager.concatenateNFAs(nfas);
            }
        case REParsingStack::GroupStartType::re_start:
            switch (type) {
                case REParsingStack::GroupStartType::parenthesis:
                    throw UnbalancedParenthesisException(m_pos);
                case REParsingStack::GroupStartType::bar:
                case REParsingStack::GroupStartType::re_start:
                    return m_stateManager.concatenateNFAs(nfas);
            }
        case REParsingStack::GroupStartType::bar: {
            NFA nfaAfterBar = m_stateManager.concatenateNFAs(nfas);
            NFA nfaBeforeBar = m_stack.popOne();
            m_stack.push(
                m_stateManager.makeAlternation(nfaBeforeBar, nfaAfterBar));
            break;
        }
        }
    }
}

void REParserImpl::parseLeftBrace() {
    const auto braceStart = m_pos;
    NFA lastNfa = checkRepetitionAndPopLastNfa();
    const auto numRepetitions = parseNumRepetitions(braceStart);
    switch (numRepetitions) {
        case 0: break;
        case 1:
            m_stack.push(lastNfa);
            break;
        default: {
            for (auto count = 0; count < numRepetitions; count++) {
                m_stack.push(m_stateManager.makeCopy(lastNfa));
            }
        }
    }
}

uint32_t REParserImpl::parseNumRepetitions(const uint32_t braceStart) {
    auto numRepetitions = 0u;
    for (advance();  // start from the next symbol after '{'
         m_sym != RIGHT_BRACE;
         advance())
    {
        if (m_pos == m_re.size()) {
            throw MissingBraceException(braceStart);
        }
        if (m_sym < '0' or m_sym > '9') {
            throw NondigitInBracesException(m_sym, m_pos);
        }
        numRepetitions = numRepetitions * 10 + (m_sym - '0');
        if (numRepetitions > MAX_BRACES_REPETITION) {
            throw TooLargeRepetitionNumberException();
        }
    }
    if (braceStart + 1 == m_pos) {
        throw EmptyBracesException(braceStart);
    }
    return numRepetitions;
}

void REParserImpl::parseRepetition() {
    NFA lastNfa = checkRepetitionAndPopLastNfa();
    switch (m_sym) {
        case KLEENE_STAR:
            m_stack.push(m_stateManager.makeKleeneClousure(lastNfa));
            break;
        case PLUS:
            m_stack.push(m_stateManager.makePlus(lastNfa));
            break;
        case QUESTION:
            m_stack.push(m_stateManager.makeQuestion(lastNfa));
            break;
        default:
            assert(false and "Unexpected repeat symbol");
    }
}

NFA REParserImpl::checkRepetitionAndPopLastNfa() {
    if (m_stack.getLastGroupStart().posInRe == m_pos - 1) {
        throw NothingToRepeatException(m_pos);
    }
    if (m_isLastStateRepetition) {
        throw MultipleRepeatException(m_pos);
    }
    return m_stack.popOne();
}

void REParserImpl::parseEscape() {
    advance();  // check the next symbol after '\'
    if (m_pos == m_re.size()) {
        throw EscapeException("Escape reaches the end of the input");
    }
    switch (m_sym) {
        case BAR:
        case LEFT_PAREN:
        case RIGHT_PAREN:
        case LEFT_BRACE:
        case RIGHT_BRACE:
        case KLEENE_STAR:
        case PLUS:
        case QUESTION:
        case ESCAPE:
            m_stack.push(m_stateManager.makeSymbol(m_sym));
            break;
        case ESCAPE_D:
            m_stack.push(m_stateManager.makeDigit());
            break;
        default:
            throw EscapeException(m_sym, m_pos);
    }
}

} // namespace RE
