//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/BeastLounge
//

#pragma once

#include <string>

namespace easyprospect
{
    namespace service
    {
        namespace shared
        {
            //class channel;
            class message;

            /// Represents a connected user
            class user_base //: public boost::enable_shared_from
            {
            public:
                std::string name;
                
            };

            class user : public user_base
            {

            };

            class ws_user : public user
            {

            };
        }
    }
}
