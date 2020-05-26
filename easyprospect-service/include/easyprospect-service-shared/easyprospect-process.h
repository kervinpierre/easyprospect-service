#pragma once
#include <easyprospect-service-shared/easyprospect-process-win.h>

namespace easyprospect
{
namespace service
{
    namespace shared
    {
        class process final
        {
            process_win pw;

          public:
            void run()
            {

                pw.setup();
                pw.listen_loop();
            }

            void send(control_worker::process_message_base& obj)
            {
                pw.send(obj);
            }
        };
    } // namespace control_worker
} // namespace service
} // namespace easyprospect