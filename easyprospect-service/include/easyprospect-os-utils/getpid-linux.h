#pragma once

namespace easyprospect
{
namespace service
{
    namespace os_utils
    {
        static long getpid_linux()
        {
            const long res = getpid();

            return res;
        }
    } // namespace os_utils
} // namespace service
} // namespace easyprospect