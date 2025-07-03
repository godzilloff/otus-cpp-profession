#include <cassert>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

#include "lib.h"
#include "ipv4.h"

int main(int /*argc*/, char const *argv[])
{
    // std::cout << "argc = " << argc << std::endl;
    // for (int i = 0; i < argc; ++i) {
    //     std::cout << "Argument " << i << ": " << argv[i] << std::endl;
    // }
    // std::cout << "Version: " << version() << std::endl;

    try
    {
        std::vector<IpV4> ip_pool;
        std::ifstream in_file{argv[1]};

        //for (std::string line; std::getline(std::cin, line);)
        for (std::string line; std::getline(in_file, line);)
        {
            ip_pool.emplace_back(line.substr(0, line.find_first_of('\t')));
        }

        std::sort(ip_pool.begin(), ip_pool.end(), IpV4::greater);

        std::for_each(ip_pool.begin(), ip_pool.end(), 
            [](auto ip){ std::cout << ip.getString() << std::endl; });

        // // filter 1
        std::for_each(ip_pool.begin(), ip_pool.end(), 
            [](auto ip){ if (ip.getValue(0) == 1)
                            std::cout << ip.getString() << std::endl; });

        // filter 46, 70
        std::for_each(ip_pool.begin(), ip_pool.end(), 
            [](auto ip){
                        if (ip.getValue(0) == 46 && ip.getValue(1) == 70)
                            std::cout << ip.getString() << std::endl; });

        // filter any 46
        std::for_each(ip_pool.begin(), ip_pool.end(), [](auto ip)
                      { 
                        if (ip.getValue(0) == 46 ||
                            ip.getValue(1) == 46 ||
                            ip.getValue(2) == 46 ||
                            ip.getValue(3) == 46)
                                std::cout << ip.getString() << std::endl; });
    }
    catch(const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
