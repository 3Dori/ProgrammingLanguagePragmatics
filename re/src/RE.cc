#include "RE.h"

#include <iostream>
#include <numeric>
#include <string_view>
#include <functional>
#include <vector>

// using std::placeholders::_1;
// using std::placeholders::_2;  TODO remove

namespace RE {

// NFA
NFANode* NodeManager::NFAFromRe(std::string_view re) {
    REParsingStack stack;
    for (auto pos = 0u; pos < re.size(); ++pos) {
        const auto sym = re[pos];
        switch (sym)
        {
        case EPS: assert(false and "Unexpected end of regex");
        case BAR: {
            // group re ahead of the bar
            auto nfas = stack.popTillLastGroupStart();
            stack.push(concatenateNFAs(nfas));
            stack.pushBar(pos);
            break;
        }
        case LEFT_PAREN:
            stack.pushOpenParen(pos);
            break;
        case RIGHT_PAREN:
            stack.push(parseLastGroup(stack, pos, true));
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
    NFA nfa = parseLastGroup(stack, 0, false);
    return nfa.startNode ? nfa.startNode : makeNFANode(true);
}

NodeManager::NFA NodeManager::parseLastGroup(REParsingStack& stack, const size_t pos, const bool matchParen) {
    while (true) {
        const auto lastGroupStartType = stack.getLastGroupStart().type;
        const auto lastGroupStartPosInRe = stack.getLastGroupStart().posInRe;
        auto nfas = stack.popTillLastGroupStart();
        switch (lastGroupStartType) {
            case REParsingStack::Pos_t::Type::open_parenthesis:
                if (matchParen) {
                    return concatenateNFAs(nfas);
                }
                else {
                    throw MissingParenthsisException(lastGroupStartPosInRe);
                }
            case REParsingStack::Pos_t::Type::re_start:
                if (matchParen) {
                    throw UnbalancedParenthesisException(pos);
                }
                else {
                    return concatenateNFAs(nfas);
                }
            case REParsingStack::Pos_t::Type::bar: {
                NFA nfa = concatenateNFAs(nfas);
                NFA lastNfa = stack.popOne();
                stack.push(makeAlternation(lastNfa, nfa));
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
    if (m_NFAs.size() >= MAX_NFA_NODE_NUM) {
        throw NFANumLimitExceededExpection();
    }
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
DFANode* NodeManager::DFAFromNFA(NFANode* nfa) {
    DFANode* dfa = makeDFANode(nfa);
    makeDFATransitions(dfa);
    return dfa;
}

DFANode* NodeManager::makeDFANode(const std::set<NFANode const*>& nfaNodes) {
    DFANode dfaNode;
    for (auto const* nfaNode : nfaNodes) {
        dfaNode.bypassEPS(nfaNode);
    }

    // create a DFA node if it doesn't exist
    const auto NFANodesInvolved = dfaNode.m_NFANodes;
    if (m_DFAs.find(NFANodesInvolved) == m_DFAs.end()) {
        m_DFAs[NFANodesInvolved] = std::move(dfaNode);
    }
    return getDFANode(NFANodesInvolved);
}

DFANode* NodeManager::getDFANode(const NFASet& NFAs) {
    return &(m_DFAs.at(NFAs));
}

void NodeManager::makeDFATransitions(DFANode* dfaNode) {
    for (auto const* nfaNode : dfaNode->m_NFANodeSet) {
        for (const auto& [sym, tos] : nfaNode->m_transitions) {
            if (sym != EPS and not dfaNode->hasTransition(sym)) {
                DFANode* nextDfaNode = makeDFANode(tos);
                dfaNode->addTransition(sym, nextDfaNode);
                makeDFATransitions(nextDfaNode);
            }
        }
    }
}

REParser::REParser(std::string_view re) {
    NFANode* nfa = m_nodeManager.NFAFromRe(re);
    m_dfa = m_nodeManager.DFAFromNFA(nfa);
}

bool REParser::matchExact(std::string_view str) const {
    assert(m_dfa != nullptr);
    return m_dfa->accept(str);
}

int32_t REParser::find(std::string_view str) const {
    return -1;
}

} // namespace RE
