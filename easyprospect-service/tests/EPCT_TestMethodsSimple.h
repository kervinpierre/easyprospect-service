#pragma once
#include <string>

namespace EPCT
{
    class EPCT_TestMethodsSimple
    {
    public:
        static int f1(int const& x);
        static int f2(int const& x, int y);
        static std::string str_func(std::string s1, std::string s2);
        static float f3(int i, float j);
        static int f4(int i, int j, int k);
    };
}

