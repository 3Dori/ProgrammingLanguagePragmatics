#include <cstdint>
#include <string_view>
#include <memory>

namespace RE
{

class DFA;
class StateManager;

class REParser {
public:
    REParser(std::string_view);
    ~REParser();

    bool matchExact(std::string_view) const;
    int32_t find(std::string_view) const;

private:
    std::unique_ptr<StateManager> m_stateManager;
    std::unique_ptr<DFA> m_dfa;
};

} // namespace RE
