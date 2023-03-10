#include <cstdint>
#include <string_view>
#include <memory>

namespace RE
{

class NFANode;
class DFANodeFromNFA;
class NodeManager;

class REParser {
public:
    REParser(std::string_view);
    ~REParser();

    bool matchExact(std::string_view) const;
    int32_t find(std::string_view) const;

private:
    std::unique_ptr<NodeManager> m_nodeManager;
    DFANodeFromNFA* m_dfa = nullptr;
};

} // namespace RE
