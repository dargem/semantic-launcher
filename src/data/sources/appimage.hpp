#include "src/data/sources/i_aggregate.hpp"

class AppImage : public IAggregate
{
public:
    bool check_applicable() const override;
    void aggregate(siv::Vector<File>& files, std::unordered_map<std::string, siv::ID> membership) const override;

    ~AppImage() = default;

private:
};