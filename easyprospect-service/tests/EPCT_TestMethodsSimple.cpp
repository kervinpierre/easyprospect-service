#include <string>
#include "EPCT_TestMethodsSimple.h"

int EPCT::EPCT_TestMethodsSimple::f1(int const& x)
{
    return x;
}

int EPCT::EPCT_TestMethodsSimple::f2(int const& x, int y)
{
    return x + y;
}

std::string EPCT::EPCT_TestMethodsSimple::str_func(std::string s1, std::string s2)
{
    return s1 + s2;
}

float EPCT::EPCT_TestMethodsSimple::f3(int i, float j)
{
    float k = j * i;
    return k;
}

int EPCT::EPCT_TestMethodsSimple::f4(int i, int j, int k)
{
    return i + j + k + f2(j, k);
}