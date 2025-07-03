#pragma once

#include <string>
#include <array>
#include <sstream>

class IpV4
{
public:
    IpV4() = default;
    explicit IpV4(std::string stringParam);
    static bool greater(const IpV4 &lhs, const IpV4 &rhs);
    std::string getString(void);
    int getValue(int index);
    ~IpV4();

private:
    std::string string;
    std::array<int, 4> data;
};

