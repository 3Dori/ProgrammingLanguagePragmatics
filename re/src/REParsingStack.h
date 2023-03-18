#pragma once

#include "FA.h"

#include <vector>

namespace RE
{

class REParsingStack {
    friend class REParserImpl;

    using Stack_t = std::vector<NFA>;

private:
    enum class GroupStartType {
        bar = 0u,
        parenthesis = 1u,
        re_start = 2u,
    };

    /**
     * predecence: re_start > parenthesis > bar
     */
    static bool hasHigherOrEqualPredecence(const GroupStartType a, const GroupStartType b) {
        return static_cast<uint32_t>(a) >= static_cast<uint32_t>(b);
    }

    struct GroupStart {
        const size_t posInStack;
        const int32_t posInRe;
        GroupStartType type;
    };

    const GroupStart& getLastGroupStart() const {
        return m_groupStarts.back();
    }

    bool isEmpty() const { return m_stack.empty(); }
    void push(const NFA& nfa) { m_stack.push_back(nfa); }

    void pushOpenParen(const int32_t posInRe) {
        m_groupStarts.push_back({m_stack.size(), posInRe, GroupStartType::parenthesis});
    }

    void pushBar(const int32_t posInRe) {
        m_groupStarts.push_back({m_stack.size(), posInRe, GroupStartType::bar});
    }

    NFA popOne();
    Stack_t popTillLastGroupStart(const GroupStartType);

private:
    Stack_t m_stack;
    std::vector<GroupStart> m_groupStarts{{0u, -1, GroupStartType::re_start}};
};

} // namespace RE
