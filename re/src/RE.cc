#include "FA.h"
#include "REDef.h"
#include "REUtility.h"

#include <REExceptions.h>
#include <RE.h>

#include <iostream>
#include <numeric>
#include <string_view>
#include <functional>
#include <vector>

// using std::placeholders::_1;
// using std::placeholders::_2;  TODO remove

namespace RE {

REParser::REParser(std::string_view re) :
    m_nodeManager(new NodeManager())
{
    NFANode* nfa = m_nodeManager->NFAFromRe(re);
    m_dfa = m_nodeManager->DFAFromNFA(nfa);
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
