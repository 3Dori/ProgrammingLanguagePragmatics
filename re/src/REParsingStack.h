#pragma once

#include "FA.h"
#include "REDef.h"
#include "StateManager.h"

#include <vector>
#include <list>
#include <map>
#include <memory>
#include <set>

namespace RE
{

class REParsingStack {
    using Stack_t = std::vector<StateManager::NFA>;

public:
    struct GroupStart {
        const size_t posInStack;
        const int32_t posInRe;
        StateManager::GroupStartType type;
    };

    inline const GroupStart& getLastGroupStart() const {
        return m_groupStarts.back();
    }

    inline bool isEmpty() const {
        return m_stack.empty();
    }

    inline void push(const StateManager::NFA& nfa) {
        m_stack.push_back(nfa);
    }

    inline void pushOpenParen(const int32_t posInRe) {
        m_groupStarts.push_back({m_stack.size(), posInRe, StateManager::GroupStartType::parenthesis});
    }

    inline void pushBar(const int32_t posInRe) {
        m_groupStarts.push_back({m_stack.size(), posInRe, StateManager::GroupStartType::bar});
    }

    inline StateManager::NFA popOne() {
        const auto ret = m_stack.back();
        m_stack.pop_back();
        return ret;
    }

    inline Stack_t popTillLastGroupStart(const StateManager::GroupStartType type) {
        // Pop last group start
        const auto lastGroupStartPosInStack = getLastGroupStart().posInStack;
        if (StateManager::hasHigherOrEqualPredecence(type, getLastGroupStart().type)) {
            m_groupStarts.pop_back();
        }
        // Pop nfas till last group start
        const auto ret = Stack_t(m_stack.begin() + lastGroupStartPosInStack, m_stack.end());
        m_stack.resize(lastGroupStartPosInStack);

        return ret;
    }
    
private:
    Stack_t m_stack;
    std::vector<GroupStart> m_groupStarts{{0u, -1, StateManager::GroupStartType::re_start}};
};

} // namespace RE
