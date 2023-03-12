#include "REDef.h"
#include "StateManager.h"
#include "REParsingStack.h"
#include <REExceptions.h>

#include <cassert>

namespace RE
{

// NFA

NFAState* StateManager::NFAFromRe(std::string_view re) {
    REParsingStack stack;
    bool isLastStateRepetition = false;
    for (auto pos = 0u; pos < re.size(); ++pos) {
        const auto sym = re[pos];
        switch (sym)
        {
        case EPS: assert(false and "Unexpected end of regex");
        case BAR:
            stack.push(parseLastGroup(stack, pos, GroupStartType::bar));
            stack.pushBar(pos);
            break;
        case LEFT_PAREN:
            stack.pushOpenParen(pos);
            break;
        case RIGHT_PAREN:
            stack.push(parseLastGroup(stack, pos, GroupStartType::parenthesis));
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
            
            NFA lastNfa = checkRepetitionAndPopLastNfa(stack, braceStart, isLastStateRepetition);
            switch (numRepetitions) {
                case 0:
                    break;
                case 1:
                    stack.push(lastNfa);
                    break;
                default: {
                    for (auto count = 0; count < numRepetitions; count++) {
                        stack.push(makeCopy(lastNfa));
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
            NFA lastNfa = checkRepetitionAndPopLastNfa(stack, pos, isLastStateRepetition);
            switch (sym) {
                case KLEENE_STAR:
                    stack.push(makeKleeneClousure(lastNfa));
                    break;
                case PLUS:
                    stack.push(makePlus(lastNfa));
                    break;
                case QUESTION:
                    stack.push(makeQuestion(lastNfa));
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
                    stack.push(makeSymbol(nextSym));
                    break;
                default:
                    throw EscapeException(nextSym, pos);
            }
            break;
        }
        default:
            stack.push(makeSymbol(sym));
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
    NFA nfa = parseLastGroup(stack, 0, GroupStartType::re_start);
    return nfa.startState ? nfa.startState : makeNFAState(true);
}

StateManager::NFA StateManager::parseLastGroup(REParsingStack& stack, const size_t pos, const GroupStartType type) {
    while (true) {
        const auto lastGroupStartType = stack.getLastGroupStart().type;
        const auto lastGroupStartPosInRe = stack.getLastGroupStart().posInRe;
        auto nfas = stack.popTillLastGroupStart(type);
        switch (lastGroupStartType) {
            case GroupStartType::parenthesis:
                switch (type) {
                    case GroupStartType::re_start:
                        throw MissingParenthsisException(lastGroupStartPosInRe);
                    case GroupStartType::bar:
                    case GroupStartType::parenthesis:
                        return concatenateNFAs(nfas);
                }
            case GroupStartType::re_start:
                switch (type) {
                    case GroupStartType::parenthesis:
                        throw UnbalancedParenthesisException(pos);
                    case GroupStartType::bar:
                    case GroupStartType::re_start:
                        return concatenateNFAs(nfas);
                }
            case GroupStartType::bar: {
                NFA nfaAfterBar = concatenateNFAs(nfas);
                NFA nfaBeforeBar = stack.popOne();
                stack.push(makeAlternation(nfaBeforeBar, nfaAfterBar));
                break;
            }
        }
    }
}

StateManager::NFA StateManager::concatenateNFAs(std::vector<NFA>& nfas)
{
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

StateManager::NFA StateManager::makeSymbol(const char sym) {
    auto startState = makeNFAState();
    auto endState = makeNFAState(true);
    startState->addTransition(sym, endState);
    m_inputSymbols.insert(sym);
    return { startState, endState };
}

StateManager::NFA StateManager::makeConcatenation(NFA& a, NFA& b) {
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

StateManager::NFA StateManager::makeAlternation(NFA& a, NFA& b) {
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

StateManager::NFA StateManager::checkRepetitionAndPopLastNfa(
    REParsingStack& stack, const size_t pos, const bool isLastStateRepetition) {
    if (stack.getLastGroupStart().posInRe == pos - 1) {
        throw NothingToRepeatException(pos);
    }
    if (isLastStateRepetition) {
        throw MultipleRepeatException(pos);
    }
    NFA lastNfa = stack.popOne();
    return lastNfa;
}

StateManager::NFA StateManager::makeKleeneClousure(NFA& nfa) {
    nfa.startState->addTransition(EPS, nfa.endState);
    nfa.endState->addTransition(EPS, nfa.startState);
    return { nfa.startState, nfa.endState };
}

StateManager::NFA StateManager::makePlus(NFA& nfa) {
    nfa.endState->addTransition(EPS, nfa.startState);
    return { nfa.startState, nfa.endState };
}

StateManager::NFA StateManager::makeQuestion(NFA& nfa) {
    nfa.startState->addTransition(EPS, nfa.endState);
    return { nfa.startState, nfa.endState };
}

StateManager::NFA StateManager::makeCopy(const NFA& nfa) {
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
        const auto& [keyValue, _] = m_DFAs.try_emplace(dfaInfo.nfasInvolved, m_DFAs.size(), dfaInfo.isFinal, dfaInfo.nfasInvolved);
        m_DFAsIndexed.push_back(&(keyValue->second));
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
