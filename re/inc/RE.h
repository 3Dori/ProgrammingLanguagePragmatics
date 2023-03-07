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
    };

    class REParsingStack {
        using Stack_t = std::vector<NFA>;
    public:
        struct Pos_t {
            const size_t posInStack;
            const size_t posInRe;
            enum class Type {
                open_parenthesis,
                bar,
                re_start
            } type;
        };

    private:
        Stack_t m_stack;
        std::vector<Pos_t> m_opens{{0, 0, Pos_t::Type::re_start}};

    public:
        const Pos_t& getLastOpen() const {
            return m_opens.back();
        }

        bool isEmpty() const {
            return m_stack.empty();
        }

        void push(const NFA& nfa) {
            m_stack.push_back(nfa);
        }

        void pushOpenParen(const size_t posInRe) {
            m_opens.push_back({m_stack.size(), posInRe, Pos_t::Type::open_parenthesis});
        }

        void pushBar(const size_t posInRe) {
            m_opens.push_back({m_stack.size(), posInRe, Pos_t::Type::bar});
        }

        /**
         * @brief Pop the last NFA in the stack
         *   Should be only invoked in NodeManager::makeRepeat
         * @return NFA 
         */
        NFA popOne() {
            const auto ret = m_stack.back();
            m_stack.pop_back();
            assert(m_opens.back().posInStack != m_stack.size() and "Unexpected open symbol when popping");
            return ret;
        }

        // TODO check: a pop must be followed by a push, except the last one
        Stack_t popTillLastOpen() {
            // Pop last open
            const auto lastOpenPos = m_opens.back().posInStack;
            m_opens.pop_back();
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
                case REParsingStack::Pos_t::Type::bar: {
                    NFA nfa = concatenateNFAs(nfas);
                    NFA lastNfa = stack.popOne();
                    stack.push(makeAlternation(lastNfa, nfa));
                    break;
                }
                case REParsingStack::Pos_t::Type::re_start:
                    return stack.popOne();
            }
        }
    }

    // TODO refactor with finishParsing
    NFA matchLastOpenParen(REParsingStack& stack, const size_t pos) {
        while (true) {
            const auto lastOpenType = stack.getLastOpen().type;
            auto nfas = stack.popTillLastOpen();
            switch (lastOpenType) {
                case REParsingStack::Pos_t::Type::open_parenthesis:
                    return concatenateNFAs(nfas);
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
