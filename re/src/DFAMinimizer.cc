#include "DFAMinimizer.h"

#include <iostream>

namespace RE
{

// DFAMinimizer
DFAMinimizer::DFAMinimizer(const std::vector<DFANodeFromNFA const*>& DFAsIndexed) :
    m_DFAToMergedDFA(std::vector<int32_t>(DFAsIndexed.size(), -1))
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
    if (nonFinals.dfaNodes.empty()) {
        m_mergedDfaNodes.erase(NON_FINALS);
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
        assert(dfaNode->hasTransition(sym));
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
    std::cout << minimizedDFA->m_nodes.size() << std::endl;
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
