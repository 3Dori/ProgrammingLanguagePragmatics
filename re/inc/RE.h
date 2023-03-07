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
        const Pos_t& getLastOpen() const {
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

        /**
         * @brief Pop the last NFA in the stack
         *   Should be only invoked in NodeManager::makeRepeat
         * @return NFA 
         */
        NFA popOne() {
            assert(not m_stack.empty() and "Stack cannot be empty for popping");
            const auto ret = m_stack.back();
            m_stack.pop_back();
            return ret;
        }

        // TODO check: a pop must be followed by a push, except the last one
        Stack_t popTillLastOpen() {
            // Pop last open
            const auto lastOpenPos = getLastOpen().posInStack;
            if (getLastOpen().type != Pos_t::Type::re_start) {
                m_groupStarts.pop_back();
            }
            // Pop nfas till last open
            const auto ret = Stack_t(m_stack.begin() + lastOpenPos, m_stack.end());
            m_stack.resize(lastOpenPos);

            return ret;
        }

        Stack_t popAll() {
            // if (not m_openParens.empty()) {
            //     throw MissingParenthsisException(m_openParens.back().posInRe);
            // }
            const auto ret = m_stack;
            m_stack.clear();
            return ret;
        }

        // 
    };

private:
    DFANode* DFAFromNFA(NFANode*);  // TODO simplify DFA
    NFANode* NFAFromRe(std::string_view);

    NFA finishParsing(REParsingStack& stack) {
        while (true) {
            const auto lastOpenType = stack.getLastOpen().type;
            const auto lastOpenPosInRe = stack.getLastOpen().posInRe;
            auto nfas = stack.popTillLastOpen();
            switch (lastOpenType) {
                case REParsingStack::Pos_t::Type::open_parenthesis:
                    throw MissingParenthsisException(lastOpenPosInRe);
                    break;
                case REParsingStack::Pos_t::Type::bar: {
                    NFA nfa = concatenateNFAs(nfas);
                    NFA lastNfa = stack.popOne();
                    stack.push(makeAlternation(lastNfa, nfa));
                    break;
                }
                case REParsingStack::Pos_t::Type::re_start:
                    return concatenateNFAs(nfas);
            }
        }
    }

    // TODO refactor with finishParsing
    void matchLastOpenParen(REParsingStack& stack, const size_t pos) {
        while (true) {
            const auto lastOpenType = stack.getLastOpen().type;
            auto nfas = stack.popTillLastOpen();
            switch (lastOpenType) {
                case REParsingStack::Pos_t::Type::open_parenthesis:
                    stack.push(concatenateNFAs(nfas));
                    return;
                case REParsingStack::Pos_t::Type::bar: {
                    NFA nfa = concatenateNFAs(nfas);
                    NFA lastNfa = stack.popOne();
                    stack.push(makeAlternation(lastNfa, nfa));
                    break;
                }
                case REParsingStack::Pos_t::Type::re_start:
                    throw UnbalancedParenthesisException(pos);
            }
        }
    }

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
