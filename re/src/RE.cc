#include "FA.h"
#include "REDef.h"
#include "REParsingStack.h"
#include "StateManager.h"
#include "DFAMinimizer.h"

#include <REExceptions.h>
#include <RE.h>

#include <iostream>
#include <numeric>
#include <string_view>
#include <functional>
#include <vector>

namespace RE {

REParser::REParser(std::string_view re) :
    m_stateManager(new StateManager())
{
    NFAState* nfa = m_stateManager->NFAFromRe(re);
    DFAStateFromNFA* dfa = m_stateManager->DFAFromNFA(nfa);
    m_dfa = DFAMinimizer(m_stateManager).minimize();
}

REParser::~REParser() = default;

bool REParser::matchExact(std::string_view str) const {
    assert(m_dfa != nullptr);
    return m_dfa->accept(str);
}

int32_t REParser::find(std::string_view str) const {
    return -1;
}

} // namespace RE
