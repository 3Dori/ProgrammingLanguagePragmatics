#include "REDef.h"
#include "NodeManager.h"
#include "REParsingStack.h"
#include <REExceptions.h>

#include <cassert>

namespace RE
{

// NFA

NFANode* NodeManager::NFAFromRe(std::string_view re) {
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
    return nfa.startNode ? nfa.startNode : makeNFANode(true);
}

NodeManager::NFA NodeManager::parseLastGroup(REParsingStack& stack, const size_t pos, const GroupStartType type) {
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

NodeManager::NFA NodeManager::checkRepetitionAndPopLastNfa(
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

NodeManager::NFA NodeManager::concatenateNFAs(std::vector<NFA>& nfas)
{
    // TODO investigate it
    // NFA emptyNfa;
    // NFA nfa = std::accumulate(nfas.begin(), nfas.end(), emptyNfa,
                            //   std::bind(&NodeManager::makeConcatenation, this, _1, _2));
                            //   [this](NFA& a, NFA& b) {
                            //       return makeConcatenation(a, b);
                            //   });
    NFA resultNfa;
    for (auto& nfa : nfas) {
        resultNfa = makeConcatenation(resultNfa, nfa);
    }
    return { resultNfa.startNode, resultNfa.endNode, NFA::Type::concatenation };
}

NFANode* NodeManager::makeNFANode(const bool isFinal) {
    m_NFAs.emplace_back(m_NFAs.size(), isFinal);
    assert(m_NFAs.back().m_id == m_NFAs.size() - 1 and "Wrong NFANode id");
    return &(m_NFAs.back());
}

NodeManager::NFA NodeManager::makeSymbol(const char sym) {
    auto startNode = makeNFANode();
    auto endNode = makeNFANode(true);
    startNode->addTransition(sym, endNode);
    m_inputSymbols.insert(sym);
    return { startNode, endNode, NFA::Type::symbol };
}

NodeManager::NFA NodeManager::makeConcatenation(NFA& a, NFA& b) {
    if (a.isEmpty()) {
        return b;
    }
    if (b.isEmpty()) {
        return a;
    }
    a.endNode->m_isFinal = false;
    a.endNode->addTransition(EPS, b.startNode);
    return { a.startNode, b.endNode, NFA::Type::concatenation };
}

NodeManager::NFA NodeManager::makeAlternation(NFA& a, NFA& b) {
    if (a.isEmpty() and b.isEmpty()) {
        return a;
    }
    else if (a.isEmpty()) {
        return makeQuestion(b);
    }
    else if (b.isEmpty()) {
        return makeQuestion(a);
    }

    auto startNode = makeNFANode();
    auto endNode = makeNFANode(true);

    a.endNode->m_isFinal = false;
    b.endNode->m_isFinal = false;

    startNode->addTransition(EPS, a.startNode);
    startNode->addTransition(EPS, b.startNode);

    a.endNode->addTransition(EPS, endNode);
    b.endNode->addTransition(EPS, endNode);

    return { startNode, endNode, NFA::Type::alternation };
}

NodeManager::NFA NodeManager::makeKleeneClousure(NFA& nfa) {
    nfa.startNode->addTransition(EPS, nfa.endNode);
    nfa.endNode->addTransition(EPS, nfa.startNode);
    return { nfa.startNode, nfa.endNode, NFA::Type::repeat };
}

NodeManager::NFA NodeManager::makePlus(NFA& nfa) {
    nfa.endNode->addTransition(EPS, nfa.startNode);
    return { nfa.startNode, nfa.endNode, NFA::Type::repeat };
}

NodeManager::NFA NodeManager::makeQuestion(NFA& nfa) {
    nfa.startNode->addTransition(EPS, nfa.endNode);
    return { nfa.startNode, nfa.endNode, NFA::Type::repeat };
}

NodeManager::NFA NodeManager::makeCopy(const NFA& nfa) {
    if (nfa.isEmpty()) {
        return nfa;
    }
    std::map<NFANode const*, NFANode*> copied;
    copied[nfa.startNode] = makeNFANode();  // TODO check valid /(|){1}/
    copyTransitions(nfa.startNode, copied);
    return { copied.at(nfa.startNode), copied.at(nfa.endNode), nfa.type };
}

void NodeManager::copyTransitions(NFANode const* copyFrom, std::map<NFANode const*, NFANode*>& copied) {
    for (const auto& [sym, tos] : copyFrom->m_transitions) {
        for (auto const* to : tos) {
            if (copied.find(to) == copied.end()) {
                copied[to] = makeNFANode(to->m_isFinal);
                copyTransitions(to, copied);
            }
            copied.at(copyFrom)->addTransition(sym, copied.at(to));
        }
    }
}

// DFA

DFANodeFromNFA* NodeManager::DFAFromNFA(NFANode const* nfa) {
    const auto dfaInfo = mergeEPSTransitions(nfa);
    DFANodeFromNFA* dfa = getDFANode(dfaInfo);
    generateDFATransitions(dfa);
    return dfa;
}

DFANodeFromNFA* NodeManager::getDFANode(const DFAInfo& dfaInfo) {
    const auto nfasInvolved = dfaInfo.nfasInvolved;
    if (m_DFAs.find(dfaInfo.nfasInvolved) == m_DFAs.end()) {
        const auto& [keyValue, _] = m_DFAs.try_emplace(dfaInfo.nfasInvolved, m_DFAs.size(), dfaInfo.isFinal, dfaInfo.nfasInvolved);
        m_DFAsIndexed.push_back(&(keyValue->second));
    }
    return &(m_DFAs.at(nfasInvolved));
}

void NodeManager::generateDFATransitions(DFANodeFromNFA* dfaNode) {
    for (const auto sym : m_inputSymbols) {
        assert(sym != EPS);
        if (dfaNode->hasTransition(sym)) {
            continue;
        }
        const auto dfaInfo = mergeTransitions(dfaNode, sym);
        DFANodeFromNFA* to = getDFANode(dfaInfo);
        dfaNode->addTransition(sym, to);
        generateDFATransitions(to);
    }
}

NodeManager::DFAInfo NodeManager::mergeEPSTransitions(NFANode const* nfaNode) {
    DFAInfo dfaInfo;
    mergeEPSTransitions(nfaNode, dfaInfo);
    return dfaInfo;
}

void NodeManager::mergeEPSTransitions(NFANode const* nfaNode, DFAInfo& dfaInfo) {
    if (dfaInfo.containsNfaNode(nfaNode)) {
        return;
    }
    dfaInfo.addNfaNode(nfaNode);
    if (nfaNode->m_isFinal) {
        dfaInfo.isFinal = true;
    }
    if (not nfaNode->hasTransition(EPS)) {
        return;
    }
    for (auto const* to : nfaNode->m_transitions.at(EPS)) {
        mergeEPSTransitions(to, dfaInfo);
    }
}

NodeManager::DFAInfo NodeManager::mergeTransitions(DFANodeFromNFA const* dfaNode, const char sym) {
    DFAInfo dfaInfo;
    for (auto const* nfaNode : dfaNode->m_NFANodeSet) {
        if (nfaNode->hasTransition(sym)) {
            for (auto const* to : nfaNode->m_transitions.at(sym)) {
                mergeEPSTransitions(to, dfaInfo);
            }
        }
    }
    return dfaInfo;
}

} // namespace RE
