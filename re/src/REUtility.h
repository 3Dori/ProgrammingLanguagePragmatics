#pragma once

#include "FA.h"
#include "REDef.h"

#include <vector>
#include <list>
#include <map>
#include <memory>
#include <set>

namespace RE
{

class REParsingStack;

/**
 * Manages the life cycle of NFA and DFA nodes
 */
class NodeManager {
    friend class REParser;

public:
    struct NFA {
        NFANode* startNode = nullptr;
        NFANode* endNode = nullptr;
        enum class Type {
            symbol,
            concatenation,
            repeat,
            alternation,
            unimplemented
        } type;

        bool isEmpty() const {
            return startNode == nullptr;
        }
    };

    enum class GroupStartType {
        bar = 0u,
        parenthesis = 1u,
        re_start = 2u,
    };

    /**
     * predecence: re_start > parenthesis > bar
    */
    inline static bool hasHigherOrEqualPredecence(const GroupStartType a, const GroupStartType b) {
        return static_cast<uint32_t>(a) >= static_cast<uint32_t>(b);
    }

private:
    // NFA
    NFANode* NFAFromRe(std::string_view);
    /**
     * Pop the parsing stack and push the NFA representing a group until:
     * switch (type)
     *   GroupStartType::parenthesis:  the last open parenthsis
     *   GroupStartType::re_start:     the bottom of the stack
     *   GroupStartType::bar:          the last open parenthsis or the bottom of the stack
    */
    NFA parseLastGroup(REParsingStack&, const size_t, const GroupStartType);

    NFA concatenateNFAs(std::vector<NFA>&);

    NFANode* makeNFANode(const bool isFinal = false);
    NFA makeSymbol(const char);
    NFA makeConcatenation(NFA&, NFA&);
    NFA makeAlternation(NFA&, NFA&);

    NFA makeKleeneClousure(NFA&);
    NFA makePlus(NFA&);
    NFA makeQuestion(NFA&);

    // DFA
    DFANodeFromNFA* DFAFromNFA(NFANode*);
    DFANodeFromNFA* getDFANode(const std::set<NFANode const*>&);
    inline DFANodeFromNFA* getDFANode(NFANode const* nfaNode) {
        return getDFANode(std::set<NFANode const*>{nfaNode});
    }

    inline DFANodeFromNFA* tryAddAndGetDFANode(DFANodeFromNFA& dfaNode) {
        const auto NFANodesInvolved = dfaNode.m_NFANodes;
        if (m_DFAs.find(NFANodesInvolved) == m_DFAs.end()) {
            m_DFAs.try_emplace(NFANodesInvolved, std::move(dfaNode));
            m_DFAsIndexed.push_back(&(m_DFAs.at(NFANodesInvolved)));
        }
        return &(m_DFAs.at(NFANodesInvolved));
    }
    void generateDFATransitions(DFANodeFromNFA*);

private:
    /**
     * Use STL containers to automatically manage resourses
     * and remove the need to use smart pointers, which could
     * produce circular references.
     * 
     * Be wary of container resizing, at which time the Nodes
     * are copied and the pointers to the nodes (in transitions)
     * are invalidated, resulting in undefined behaviors.
     */
    std::list<NFANode> m_NFAs;          // unlike vector, lists are never resized
    std::map<NodeSet, DFANodeFromNFA> m_DFAs;   // unlike unordered_map, maps are never resized

    std::vector<DFANodeFromNFA const*> m_DFAsIndexed;
};

class DFAMinimizer {
public:
    struct MergedDfaNode {
        MergedDfaNode(const int32_t id, const bool isFinal)
            : id(id), isFinal(isFinal) {}
        int32_t id;
        bool isFinal;
        std::set<DFANodeFromNFA const*> dfaNodes;
    };
    using MergedDfaNodes_it = std::list<MergedDfaNode>::iterator;

public:
    DFAMinimizer(const std::vector<DFANodeFromNFA const*>&);
    std::unique_ptr<DFA> minimize();

private:
    MergedDfaNode* makeMergedDfaNode(const bool);
    void splitMergedDfaNodes(const MergedDfaNode&, const char);
    char searchForAmbiguousSymbol(const MergedDfaNode&) const;
    std::unique_ptr<DFA> constructMinimizedDFA() const;
    void mergeTransitions(const MergedDfaNode&, DFA&) const;

private:
    std::vector<int32_t> m_DFAToMergedDFA;

    std::map<int32_t, MergedDfaNode> m_mergedDfaNodes;  // fixed capacity
    int32_t m_mergedDfaNodesId = 0;
};

class REParsingStack {
    using Stack_t = std::vector<NodeManager::NFA>;

public:
    struct GroupStart {
        const size_t posInStack;
        const int32_t posInRe;
        NodeManager::GroupStartType type;
    };

    inline const GroupStart& getLastGroupStart() const {
        return m_groupStarts.back();
    }

    inline bool isEmpty() const {
        return m_stack.empty();
    }

    inline void push(const NodeManager::NFA& nfa) {
        m_stack.push_back(nfa);
    }

    inline void pushOpenParen(const int32_t posInRe) {
        m_groupStarts.push_back({m_stack.size(), posInRe, NodeManager::GroupStartType::parenthesis});
    }

    inline void pushBar(const int32_t posInRe) {
        m_groupStarts.push_back({m_stack.size(), posInRe, NodeManager::GroupStartType::bar});
    }

    inline NodeManager::NFA popOne() {
        const auto ret = m_stack.back();
        m_stack.pop_back();
        return ret;
    }

    inline Stack_t popTillLastGroupStart(const NodeManager::GroupStartType type) {
        // Pop last group start
        const auto lastGroupStartPosInStack = getLastGroupStart().posInStack;
        if (NodeManager::hasHigherOrEqualPredecence(type, getLastGroupStart().type)) {
            m_groupStarts.pop_back();
        }
        // Pop nfas till last group start
        const auto ret = Stack_t(m_stack.begin() + lastGroupStartPosInStack, m_stack.end());
        m_stack.resize(lastGroupStartPosInStack);

        return ret;
    }
    
private:
    Stack_t m_stack;
    std::vector<GroupStart> m_groupStarts{{0u, -1, NodeManager::GroupStartType::re_start}};
};

} // namespace RE
