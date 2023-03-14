#include "REParserImpl.h"

#include <RE.h>

#include <string_view>

namespace RE {

REParser::REParser(std::string_view re) :
    m_parser(new REParserImpl(re)) {}

REParser::~REParser() = default;

bool REParser::matchExact(std::string_view str) const {
    return m_parser->matchExact(str);
}

int32_t REParser::find(std::string_view str) const {
    return -1;
}

} // namespace RE
