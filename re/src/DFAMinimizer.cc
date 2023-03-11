#include "DFAMinimizer.h"

namespace RE
{

// DFAMinimizer
DFAMinimizer::DFAMinimizer(const std::vector<DFANodeFromNFA const*>& DFAsIndexed) :
    m_DFAToMergedDFA(std::vector<int32_t>(DFAsIndexed.size(), -1)),
    m_deadState(DFAsIndexed.back())
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
            removeDeadState();
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
    for (auto const* dfaNode : node.dfaNodes) {
        auto const* toNode = dfaNode->m_transitions.at(sym);
        int32_t to = m_DFAToMergedDFA[toNode->m_id];

        if (newTransitions.find(to) == newTransitions.end()) {
            newTransitions[to] = makeMergedDfaNode(node.isFinal);
        }
        newTransitions[to]->dfaNodes.insert(dfaNode);
        m_DFAToMergedDFA[dfaNode->m_id] = newTransitions[to]->id;
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

void DFAMinimizer::removeDeadState() {
    const auto deadState = m_DFAToMergedDFA.back();
    assert(m_mergedDfaNodes.at(deadState).dfaNodes.size() == 1);
    m_mergedDfaNodes.erase(deadState);
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
            if (not node.hasTransition(sym) and to != m_deadState) {
                auto const mergedTo = m_DFAToMergedDFA[to->m_id];
                node.addTransition(sym, &(dfa.m_nodes.at(mergedTo)));
            }
        }
    }
}

} // namespace RE
