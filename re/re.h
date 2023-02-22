#pragma once

#include <vector>
#include <memory>
#include <string>
#include <exception>

namespace Re {

static constexpr char EPS = 0;

class ReException : public std::runtime_error {
public:
    explicit ReException(const std::string& message) : std::runtime_error(message) {}
};

class ParenthesisMatchingException : public ReException {
public:
    explicit ParenthesisMatchingException(const std::string& message) : ReException(message) {}
};

struct NFANode;
using NFANodeSP = std::shared_ptr<NFANode>;

struct Transition {
    Transition(const char sym, NFANodeSP to) : sym(sym), to(to) {}

    char sym;
    NFANodeSP to;
};

struct NFANode {
    std::vector<Transition> transitions;

    void addTransition(const char sym, NFANodeSP to = nullptr) {
        transitions.emplace_back(sym, to);
    }

    static NFANodeSP constructNFA(const std::string& re) {
        return constructNFA(re.c_str(), 0, re.size());
    }

private:
    static NFANodeSP constructNFA(const char*, uint32_t, uint32_t);
    static uint32_t findRightParen(const char*, uint32_t);

private:
    // TODO support escape
    static constexpr char LEFT_PAREN = '(';
    static constexpr char RIGHT_PAREN = ')';
    static constexpr char BAR = '|';
    static constexpr char KLEENE_STAR = '*';
};

}