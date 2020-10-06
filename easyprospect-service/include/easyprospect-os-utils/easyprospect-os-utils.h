#pragma once
#include <iomanip>
#include <string>
#include <sstream>
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

            static std::string binary_to_string(const unsigned char buffer[], const std::size_t size, const unsigned int address_max=0)
            {
                unsigned long address = 0;

                std::stringstream ostr;

                ostr << std::hex << std::setfill('0');

                for(std::size_t i=0; i<size; i++)
                {
                    int  nread;
                    char buf[16] = {0};

                    for (nread = 0; nread < 16 && i<size; nread++)
                        buf[nread] = buffer[i++];

                    if (nread == 0)
                        break;

                    // Show the address
                    ostr << std::setw(8) << address;

                    // Show the hex codes
                    for (auto j = 0; j < 16; j++)
                    {
                        if (j % 8 == 0)
                            ostr << ' ';
                        if (j < nread)
                            ostr << ' ' << std::setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(buf[j]));
                        else
                            ostr << "   ";
                    }

                    // Show printable characters
                    ostr << "  ";
                    for (auto j = 0; j < nread; j++)
                    {
                        if (buf[j] < 32)
                            ostr << '.';
                        else
                            ostr << buf[j];
                    }

                    ostr << "\n";
                    address += 16;

                    if ( address_max > 0 && address >= address_max )
                    {
                        break;
                    }
                }

                return ostr.str();
            }
        };
    } // namespace os_utils
} // namespace service
} // namespace easyprospect