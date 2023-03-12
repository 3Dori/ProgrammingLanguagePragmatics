#pragma once

#include <stdexcept>
#include <string>


namespace RE {

class REException : public std::runtime_error {
public:
    explicit REException(const std::string& message) : std::runtime_error(message) {}
};

class UnbalancedParenthesisException : public REException {
public:
    explicit UnbalancedParenthesisException(const size_t pos) :
        REException("Unbalanced parenthesis at position " + std::to_string(pos))
    {}
};

class MissingParenthsisException : public REException {
public:
    explicit MissingParenthsisException(const size_t posOpenParen) : 
        REException("Missing parenthesis, unterminated open parenthsis at position " +
                    std::to_string(posOpenParen))
    {}
};

class UnbalancedBraceException : public REException {
public:
    explicit UnbalancedBraceException(const size_t pos) :
        REException("Unbalanced brace at position " + std::to_string(pos))
    {}
};

class MissingBraceException : public REException {
public:
    explicit MissingBraceException(const size_t posOpenParen) : 
        REException("Missing brace, unterminated open brace at position " +
                    std::to_string(posOpenParen))
    {}
};

class NondigitInBracesException : public REException {
public:
    explicit NondigitInBracesException(const char sym, const size_t pos) : 
        REException(std::string("Unexpected symbol [") + sym +
                    "] in braces at position " + std::to_string(pos))
    {}
};

class EmptyBracesException : public REException {
public:
    explicit EmptyBracesException(const size_t braceStart) : 
        REException("Empty pattern within braces at position " +
                    std::to_string(braceStart))
    {}
};

class TooLargeRepetitionNumberException : public REException {
public:
    explicit TooLargeRepetitionNumberException() : 
        REException("The repetition number is too large")
    {}
};

class NFANumLimitExceededExpection : public REException { // not used
public:
    explicit NFANumLimitExceededExpection() :
        REException("The limit of number of NFA nodes is exceeded")
    {}
};

class MultipleRepeatException : public REException {
public:
    explicit MultipleRepeatException(const size_t pos) :
        REException("Multiple repeat at position " + std::to_string(pos))
    {}
};

class NothingToRepeatException : public REException {
public:
    explicit NothingToRepeatException(const size_t pos) :
        REException("Nothing to repeat at position " + std::to_string(pos))
    {}
};

class EscapeException : public REException {
public:
    explicit EscapeException(const char sym, const size_t pos) :
        REException(std::string("Unexpected escape character [") + sym +
                    "] at position " + std::to_string(pos))
    {}

    explicit EscapeException(const std::string& message) :
        REException(message)
    {}
};

} // namespace RE
