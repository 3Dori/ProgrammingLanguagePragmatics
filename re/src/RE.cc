#include "REParserImpl.h"

#include <RE.h>

namespace RE {

REParser::REParser(REParser::RE_t re) : m_parser(new REParserImpl(re)) {}

REParser::~REParser() = default;

bool REParser::matchExact(REParser::Str_t str) const {
    return m_parser->matchExact(str);
}

int32_t REParser::find(REParser::Str_t) const { return -1; }

} // namespace RE
