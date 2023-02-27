#pragma once

#include <vector>
#include <set>
#include <map>
#include <memory>
#include <string>
#include <exception>

namespace Re {

class ReException : public std::runtime_error {
public:
    explicit ReException(const std::string& message) : std::runtime_error(message) {}
};

class ParenthesisMatchingException : public ReException {
public:
    explicit ParenthesisMatchingException(const std::string& message) : ReException(message) {}
};

struct NFANode;
struct DFANode;
using NFANodeSP = std::shared_ptr<NFANode>;
using DFANodeSP = std::shared_ptr<DFANode>;

// TODO loops cause memory leak
struct NFANode {
    bool isFinal;
    struct Transition {
        char sym;
        NFANodeSP to;
    };
    std::vector<Transition> transitions;

    NFANode(bool isFinal) : isFinal(isFinal) {}

    void addTransition(const char sym, const NFANodeSP& to) {
        transitions.push_back({sym, to});
    }
};

struct DFANode {
    bool isFinal;
    std::map<char, DFANodeSP> transitions;
    std::set<NFANodeSP> NFANodes;

    DFANode(bool isFinal) : isFinal(isFinal) {}

    static DFANodeSP NFAToDFA(const NFANodeSP& nfa);

private:
    void addTransition(const char sym, const NFANodeSP& to);
    void bypassEPS(const NFANodeSP&);
    void bfsNFAToDFA();
    bool hasState(const NFANodeSP&) const;
    bool hasTransition(const char) const;
};

}