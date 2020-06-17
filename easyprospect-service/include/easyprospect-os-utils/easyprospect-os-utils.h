#pragma once
#include <boost/predef.h>

#ifdef BOOST_OS_WINDOWS
#include <easyprospect-os-utils/getpid-win.h>
#else
#include <easyprospect-os-utils/getpid-linux.h>
#endif

namespace easyprospect
{
namespace service
{
    namespace os_utils
    {
        class ep_process_utils
        {
        public:
            static long getpid()
            {
                long res = 0;

#ifdef BOOST_OS_WINDOWS
                res = getpid_win();
#else
                res = getpid_linux();
#endif

                return res;
            }
        };
    } // namespace os_utils
} // namespace service
} // namespace easyprospect