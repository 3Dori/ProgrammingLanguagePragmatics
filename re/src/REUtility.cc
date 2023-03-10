#include "REUtility.h"
#include "REDef.h"
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
DFANodeFromNFA* NodeManager::DFAFromNFA(NFANode* nfa) {
    DFANodeFromNFA* dfa = getDFANode(nfa);
    generateDFATransitions(dfa);
    return dfa;
}

DFANodeFromNFA* NodeManager::getDFANode(const std::set<NFANode const*>& nfaNodes) {
    DFANodeFromNFA dfaNode(m_DFAs.size());
    for (auto const* nfaNode : nfaNodes) {
        dfaNode.mergeEPSTransition(nfaNode);
    }

    return tryAddAndGetDFANode(dfaNode);
}

void NodeManager::generateDFATransitions(DFANodeFromNFA* dfaNode) {
    for (auto const* nfaNode : dfaNode->m_NFANodeSet) {
        for (const auto& [sym, tos] : nfaNode->m_transitions) {
            if (sym != EPS and not dfaNode->hasTransition(sym)) {
                DFANodeFromNFA* nextDfaNode = getDFANode(tos);
                dfaNode->addTransition(sym, nextDfaNode);
                generateDFATransitions(nextDfaNode);
            }
        }
    }
}

// DFAMinimizer
DFAMinimizer::DFAMinimizer(const std::vector<DFANodeFromNFA const*>& DFAsIndexed) :
    m_DFAToMergedDFA(std::vector<int32_t>(-1, DFAsIndexed.size()))
{
    // merge all final states and non-final states
    constexpr auto NON_FINALS = 0u;
    constexpr auto FINALS = 1u;
    makeMergedDfaNode(NON_FINALS);
    makeMergedDfaNode(FINALS);

    auto& nonFinals = m_mergedDfaNodes.at(NON_FINALS);
    auto& finals    = m_mergedDfaNodes.at(FINALS);
    for (auto const* dfaNode : DFAsIndexed) {
        const auto id = dfaNode->m_id;
        if (dfaNode->m_isFinal) {
            finals.dfaNodes.insert(dfaNode);
            m_DFAToMergedDFA[id] = FINALS;
        }
        else {
            nonFinals.dfaNodes.insert(dfaNode);
            m_DFAToMergedDFA[id] = NON_FINALS;
        }
    }
}

std::unique_ptr<DFA> DFAMinimizer::minimize(){
    while (true) {
        bool hasAmbiguity = false;
        for (auto& [_, mergedDfaNode] : m_mergedDfaNodes) {
            if (const char sym = searchForAmbiguousSymbol(mergedDfaNode)) {  // TODO memoize ambiguous symbol
                splitMergedDfaNodes(mergedDfaNode, sym);
                hasAmbiguity = true;
                break;
            }
        }
        if (not hasAmbiguity) {
            return constructMinimizedDFA();
        }
    }
}

DFAMinimizer::MergedDfaNode* DFAMinimizer::makeMergedDfaNode(const bool isFinal) {
    const auto id = m_mergedDfaNodesId;
    m_mergedDfaNodes.try_emplace(id, id, isFinal);  // m_mergedDfaNodes[id] = MergedDfaNode(id, isFinal);
    m_mergedDfaNodesId++;
    return &(m_mergedDfaNodes.at(id));
}

void DFAMinimizer::splitMergedDfaNodes(const MergedDfaNode& node, const char sym) {
    std::map<int32_t, MergedDfaNode*> newTransitions;
    const auto id = node.id;
    constexpr auto NO_TRANSITION = -1;
    for (auto const* dfaNode : node.dfaNodes) {
        int32_t to;
        bool isFinal;
        if (not dfaNode->hasTransition(sym)) {
            to = NO_TRANSITION;
            isFinal = dfaNode->m_isFinal;
        }
        else {
            auto const* toNode = dfaNode->m_transitions.at(sym);
            to = m_DFAToMergedDFA[toNode->m_id];
            isFinal = toNode->m_isFinal;
        }

        if (newTransitions.find(to) == newTransitions.end()) {
            newTransitions[to] = makeMergedDfaNode(isFinal);
        }
        newTransitions[to]->dfaNodes.insert(dfaNode);
        m_DFAToMergedDFA[dfaNode->m_id] = newTransitions[to]->id;
    }
    for (auto const* dfaNode : node.dfaNodes) {
        if (m_DFAToMergedDFA[dfaNode->m_id] == id) {
            assert(false and "The node is split and no node should belong to it");
        }
    }
    
    m_mergedDfaNodes.erase(id);
}

char DFAMinimizer::searchForAmbiguousSymbol(const MergedDfaNode& mergedDfa) const {
    std::map<char, size_t> transitions;
    for (auto const* dfa : mergedDfa.dfaNodes) {
        for (auto [sym, to] : dfa->m_transitions) {
            const auto toMergedDfaNode = m_DFAToMergedDFA[to->m_id];
            if (transitions.find(sym) == transitions.end()) {
                transitions[sym] = toMergedDfaNode;
            }
            else if (transitions.at(sym) != toMergedDfaNode) {  // has ambiguity
                return sym;
            }
        }
    }
    return 0;
}

std::unique_ptr<DFA> DFAMinimizer::constructMinimizedDFA() const {
    auto minimizedDFA = std::make_unique<DFA>();
    for (const auto& [id, mergedDfaNode] : m_mergedDfaNodes) {
        minimizedDFA->m_nodes.try_emplace(id, static_cast<size_t>(id), mergedDfaNode.isFinal);
    }

    for (const auto& [_, mergedDfaNode] : m_mergedDfaNodes) {
        mergeTransitions(mergedDfaNode, *minimizedDFA);
    }
    const int32_t start = m_DFAToMergedDFA[0];
    minimizedDFA->setStart(start);
    return minimizedDFA;
}

void DFAMinimizer::mergeTransitions(const MergedDfaNode& from, DFA& dfa) const {
    auto& node = dfa.m_nodes.at(from.id);
    for (auto const* dfaNode : from.dfaNodes) {
        for (const auto& [sym, to] : dfaNode->m_transitions) {
            if (not node.hasTransition(sym)) {
                auto const mergedTo = m_DFAToMergedDFA[to->m_id];
                node.addTransition(sym, &(dfa.m_nodes.at(mergedTo)));
            }
            else {
                // TODO remove assert
                auto const mergedTo = m_DFAToMergedDFA[to->m_id];
                assert(node.m_transitions.at(sym) == &(dfa.m_nodes.at(mergedTo)));
            }
        }
    }
}

} // namespace RE
