#pragma once

#include "FA.h"
#include "REDef.h"

#include <vector>
#include <list>
#include <map>
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
    DFANode* DFAFromNFA(NFANode*);  // TODO simplify DFA
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

    DFANode* makeDFANode(const std::set<NFANode const*>&);

    inline DFANode* makeDFANode(NFANode const* nfaNode) {
        return makeDFANode(std::set<NFANode const*>{nfaNode});
    }

    DFANode* getDFANode(const NFASet&);
    void generateDFATransitions(DFANode*);

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
    std::map<NFASet, DFANode> m_DFAs;   // unlike unordered_map, maps are never resized
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
