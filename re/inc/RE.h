#include "REUtility.h"
#include "FA.h"

#include <string_view>
#include <vector>
#include <list>
#include <map>
#include <cstdint>

namespace RE
{

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

    class REParsingStack {
        using Stack_t = std::vector<NFA>;
    public:
        struct GroupStart {
            const size_t posInStack;
            const int32_t posInRe;
            GroupStartType type;
        };

    private:
        Stack_t m_stack;
        std::vector<GroupStart> m_groupStarts{{0u, -1, GroupStartType::re_start}};

    public:
        const GroupStart& getLastGroupStart() const {
            return m_groupStarts.back();
        }

        bool isEmpty() const {
            return m_stack.empty();
        }

        void push(const NFA& nfa) {
            m_stack.push_back(nfa);
        }

        void pushOpenParen(const int32_t posInRe) {
            m_groupStarts.push_back({m_stack.size(), posInRe, GroupStartType::parenthesis});
        }

        void pushBar(const int32_t posInRe) {
            m_groupStarts.push_back({m_stack.size(), posInRe, GroupStartType::bar});
        }

        NFA popOne() {
            const auto ret = m_stack.back();
            m_stack.pop_back();
            return ret;
        }

        Stack_t popTillLastGroupStart(const GroupStartType type) {
            // Pop last group start
            const auto lastGroupStartPosInStack = getLastGroupStart().posInStack;
            if (hasHigherOrEqualPredecence(type, getLastGroupStart().type)) {
                m_groupStarts.pop_back();
            }
            // Pop nfas till last group start
            const auto ret = Stack_t(m_stack.begin() + lastGroupStartPosInStack, m_stack.end());
            m_stack.resize(lastGroupStartPosInStack);

            return ret;
        }
    private:
        /**
         * predecence: re_start > parenthesis > bar
        */
        bool hasHigherOrEqualPredecence(const GroupStartType a, const GroupStartType b) {
            return static_cast<uint32_t>(a) >= static_cast<uint32_t>(b);
        }
    };

private:
    DFANode* DFAFromNFA(NFANode*);  // TODO simplify DFA
    NFANode* NFAFromRe(std::string_view);

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
    void makeDFATransitions(DFANode*);

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


class REParser {
public:
    REParser(std::string_view);
    bool matchExact(std::string_view) const;
    int32_t find(std::string_view) const;

private:
    NodeManager m_nodeManager;
    DFANode* m_dfa = nullptr;
};

} // namespace RE
