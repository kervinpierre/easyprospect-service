

#include "easyprospect-web-worker/system_util.h"
#include "easyprospect-web-worker/system.hpp"

void
easyprospect::service::web_worker::system_util::make_system_channel(server& srv)
{
    insert<system_channel>(
        srv.channel_list(),
        srv);
}
