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
        case KLEENE_STAR:
        case PLUS:
        case QUESTION: {
            if (stack.getLastGroupStart().posInRe + 1 == pos) {
                throw NothingToRepeatException(pos);
            }
            NFA lastNfa = stack.popOne();
            if (lastNfa.type == NFA::Type::repeat) {
                throw MultipleRepeatException(pos);
            }
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
    return {resultNfa.startNode, resultNfa.endNode, NFA::Type::concatenation};
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
    return {startNode, endNode, NFA::Type::symbol};
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
    return {a.startNode, b.endNode, NFA::Type::concatenation};
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

    return {startNode, endNode, NFA::Type::alternation};
}

NodeManager::NFA NodeManager::makeKleeneClousure(NFA& nfa) {
    nfa.startNode->addTransition(EPS, nfa.endNode);
    nfa.endNode->addTransition(EPS, nfa.startNode);
    return {nfa.startNode, nfa.endNode, NFA::Type::repeat};
}

NodeManager::NFA NodeManager::makePlus(NFA& nfa) {
    nfa.endNode->addTransition(EPS, nfa.startNode);
    return {nfa.startNode, nfa.endNode, NFA::Type::repeat};
}

NodeManager::NFA NodeManager::makeQuestion(NFA& nfa) {
    nfa.startNode->addTransition(EPS, nfa.endNode);
    return {nfa.startNode, nfa.endNode, NFA::Type::repeat};
}

// DFA
DFANodeFromNFA* NodeManager::DFAFromNFA(NFANode* nfa) {
    DFANodeFromNFA* dfa = getDFANode(nfa);
    generateDFATransitions(dfa);
    return dfa;
}

// NodeSet NodeManager::mergeEPSTransition(NFANode* nfaNode) {
//     NodeSet nfasInvolved;
//     mergeEPSTransition(nfaNode);
//     return nfasInvolved;
// }

// NodeSet NodeManager::mergeEPSTransition(NFANode* nfaNode, NodeSet& nfasInvolved) {
    
// }

DFANodeFromNFA* NodeManager::getDFANode(const NFANodeSet& nfaNodes) {
    DFANodeFromNFA dfaNode = makeDFANode();
    for (auto const* nfaNode : nfaNodes) {
        dfaNode.mergeEPSTransition(nfaNode);
    }

    return tryAddAndGetDFANode(dfaNode);
}

DFANodeFromNFA* NodeManager::tryAddAndGetDFANode(DFANodeFromNFA& dfaNode) {
    const auto NFANodesInvolved = dfaNode.m_NFANodeSet;  // TODO non-move
    if (m_DFAs.find(NFANodesInvolved) == m_DFAs.end()) {
        const auto [keyValue, _] = m_DFAs.try_emplace(NFANodesInvolved, std::move(dfaNode));
        m_DFAsIndexed.push_back(&(keyValue->second));
    }
    return &(m_DFAs.at(NFANodesInvolved));
}

void NodeManager::generateDFATransitions(DFANodeFromNFA* dfaNode) {
    for (auto const* nfaNode : dfaNode->m_NFANodeSet) {
        for (const auto& [sym, tos] : nfaNode->m_transitions) {
            if (sym != EPS and not dfaNode->hasTransition(sym)) {  // FIXME
                DFANodeFromNFA* nextDfaNode = getDFANode(tos);
                dfaNode->addTransition(sym, nextDfaNode);
                generateDFATransitions(nextDfaNode);
            }
        }
    }
}

} // namespace RE
