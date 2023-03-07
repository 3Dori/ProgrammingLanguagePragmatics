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

    class REParsingStack {
        using Stack_t = std::vector<NFA>;
    public:
        struct Pos_t {
            const size_t posInStack;
            const int32_t posInRe;
            enum class Type {
                open_parenthesis,
                bar,
                re_start
            } type;
        };

    private:
        Stack_t m_stack;
        std::vector<Pos_t> m_groupStarts{{0, -1, Pos_t::Type::re_start}};

    public:
        const Pos_t& getLastGroupStart() const {
            return m_groupStarts.back();
        }

        bool isEmpty() const {
            return m_stack.empty();
        }

        void push(const NFA& nfa) {
            m_stack.push_back(nfa);
        }

        void pushOpenParen(const int32_t posInRe) {
            m_groupStarts.push_back({m_stack.size(), posInRe, Pos_t::Type::open_parenthesis});
        }

        void pushBar(const int32_t posInRe) {
            m_groupStarts.push_back({m_stack.size(), posInRe, Pos_t::Type::bar});
        }

        NFA popOne() {
            const auto ret = m_stack.back();
            m_stack.pop_back();
            return ret;
        }

        Stack_t popTillLastGroupStart() {
            // Pop last group start
            const auto lastGroupStartPosInStack = getLastGroupStart().posInStack;
            if (getLastGroupStart().type != Pos_t::Type::re_start) {
                m_groupStarts.pop_back();
            }
            // Pop nfas till last group start
            const auto ret = Stack_t(m_stack.begin() + lastGroupStartPosInStack, m_stack.end());
            m_stack.resize(lastGroupStartPosInStack);

            return ret;
        }
    };

private:
    DFANode* DFAFromNFA(NFANode*);  // TODO simplify DFA
    NFANode* NFAFromRe(std::string_view);

    NFA parseLastGroup(REParsingStack&, const size_t, const bool);

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
