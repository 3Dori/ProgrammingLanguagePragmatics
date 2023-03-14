#include "DFAMinimizer.h"
#include "REParserImpl.h"
#include "REParsingStack.h"

#include <REExceptions.h>

#include <string_view>

namespace RE
{

REParserImpl::REParserImpl(std::string_view re) {
    NFAState* nfa = NFAFromRe(re);
    DFAStateFromNFA* dfa = m_stateManager.DFAFromNFA(nfa);
    m_dfa = DFAMinimizer(m_stateManager).minimize();
}

NFAState* REParserImpl::NFAFromRe(std::string_view re) {
    bool isLastStateRepetition = false;
    for (auto pos = 0u; pos < re.size(); ++pos) {
        const auto sym = re[pos];
        switch (sym) {
        case EPS: assert(false and "Unexpected end of regex");
        case BAR:
            m_stack.push(parseLastGroup(pos, REParsingStack::GroupStartType::bar));
            m_stack.pushBar(pos);
            break;
        case LEFT_PAREN:
            m_stack.pushOpenParen(pos);
            break;
        case RIGHT_PAREN:
            m_stack.push(parseLastGroup(pos, REParsingStack::GroupStartType::parenthesis));
            break;
        case LEFT_BRACE: {
            const auto braceStart = pos;
            auto numRepetitions = 0u;
            for (++pos; re[pos] != RIGHT_BRACE; ++pos) {
                if (pos == re.size()) {
                    throw MissingBraceException(braceStart);
                }
                const auto nextSym = re[pos];
                if (nextSym < '0' or nextSym > '9') {
                    throw NondigitInBracesException(nextSym, pos);
                }
                numRepetitions = numRepetitions * 10 + (nextSym - '0');
                if (numRepetitions > MAX_BRACES_REPETITION) {
                    throw TooLargeRepetitionNumberException();
                }
            }
            if (braceStart + 1 == pos) {
                throw EmptyBracesException(braceStart);
            }
            
            NFA lastNfa = m_stack.checkRepetitionAndPopLastNfa(braceStart, isLastStateRepetition);
            switch (numRepetitions) {
                case 0:
                    break;
                case 1:
                    m_stack.push(lastNfa);
                    break;
                default: {
                    for (auto count = 0; count < numRepetitions; count++) {
                        m_stack.push(m_stateManager.makeCopy(lastNfa));
                    }
                }
            }
            break;
        }
        case RIGHT_BRACE:
            throw UnbalancedBraceException(pos);
        case KLEENE_STAR:
        case PLUS:
        case QUESTION: {
            NFA lastNfa = m_stack.checkRepetitionAndPopLastNfa(pos, isLastStateRepetition);
            switch (sym) {
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
            break;
        }
        case ESCAPE: {
            pos++;
            if (pos == re.size()) {
                throw EscapeException("Escape reaches the end of the input");
            }
            const auto nextSym = re[pos];
            switch (nextSym) {
                case BAR:
                case LEFT_PAREN:
                case RIGHT_PAREN:
                case LEFT_BRACE:
                case RIGHT_BRACE:
                case KLEENE_STAR:
                case PLUS:
                case QUESTION:
                case ESCAPE:
                    m_stack.push(m_stateManager.makeSymbol(nextSym));
                    break;
                default:
                    throw EscapeException(nextSym, pos);
            }
            break;
        }
        default:
            m_stack.push(m_stateManager.makeSymbol(sym));
        }

        switch (sym) {
        case KLEENE_STAR:
        case PLUS:
        case QUESTION:
        case LEFT_BRACE:
            isLastStateRepetition = true;
            break;
        default:
            isLastStateRepetition = false;
        }
    }
    NFA nfa = parseLastGroup(0, REParsingStack::GroupStartType::re_start);
    return nfa.startState ? nfa.startState : m_stateManager.makeNFAState(true);
}


NFA REParserImpl::parseLastGroup(const size_t pos, const REParsingStack::GroupStartType type) {
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
                    throw UnbalancedParenthesisException(pos);
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

} // namespace RE
