#pragma once

#include <cstdint>
#include <string_view>
#include <memory>

namespace RE {

class REParserImpl;

class REParser {
public:
    using RE_t = const std::string_view&;
    using Str_t = const std::string_view&;
    REParser(RE_t);
    ~REParser();

    bool matchExact(Str_t) const;
    int32_t find(Str_t) const;

   private:
    std::unique_ptr<REParserImpl> m_parser;
};

} // namespace RE
