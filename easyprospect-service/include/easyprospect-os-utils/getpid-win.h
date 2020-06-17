#pragma once

#include <process.h>

namespace easyprospect
{
namespace service
{
    namespace os_utils
    {
        static long getpid_win()
        {
            const long res = _getpid();

            return res;
        }
    } // namespace os_utils
} // namespace service
} // namespace easyprospect