#include "IpV4.h"

IpV4::IpV4(std::string stringParam)
{

    string = stringParam;
    std::string segment;
    std::stringstream ss(stringParam);
    int i = 0;

    // Разделяем строку по символу '.'
    while (std::getline(ss, segment, '.') && i < 4)
    {
        data[i] = std::stoi(segment);
        i++;
    }
}

std::string IpV4::getString(void)
{
    return string;
}

int IpV4::getValue(int index)
{
    if (index >= 0 && index < 4)
        return data[index];
    throw std::out_of_range("Index is out of range");
}

bool IpV4::greater(const IpV4 &lhs, const IpV4 &rhs)
{
    for (size_t i = 0; i < 4; i++)
    {
        if (lhs.data[i] > rhs.data[i])
            return true;
        if (lhs.data[i] < rhs.data[i])
            return false;
    }
    return false;
}

IpV4::~IpV4()
{
}