#include "REParsingStack.h"

#include <REExceptions.h>

namespace RE {

NFA REParsingStack::popOne() {
    const auto ret = m_stack.back();
    m_stack.pop_back();
    return ret;
}

REParsingStack::Stack_t REParsingStack::popTillLastGroupStart(
    const GroupStartType type) {
    // Pop last group start
    const auto lastGroupStartPosInStack = getLastGroupStart().posInStack;
    if (hasHigherOrEqualPredecence(type, getLastGroupStart().type)) {
        m_groupStarts.pop_back();
    }
    // Pop nfas till last group start
    const auto ret =
        Stack_t(m_stack.begin() + lastGroupStartPosInStack, m_stack.end());
    m_stack.resize(lastGroupStartPosInStack);

    return ret;
}

} // namespace RE
