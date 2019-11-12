#pragma once

#ifndef BOOST_ALL_DYN_LINK
#   define BOOST_ALL_DYN_LINK
#endif

#include <string>
#include <vector>

namespace EPCT
{
    class EPCT_Utils
    {

    public:
        //Create a context, run a script and remove the context
         static bool test_util_1(std::string script);
    };
}

