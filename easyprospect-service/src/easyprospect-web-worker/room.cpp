#include <easyprospect-web-worker/room.hpp>
#include "easyprospect-web-worker/channel_list.hpp"

namespace easyprospect
{
    namespace service
    {
        namespace web_worker
        {
            void
                make_room(
                    channel_list& list,
                    beast::string_view name)
            {
                easyprospect::service::web_worker::insert<room_impl>(list, name, list);
            }
        }
    }
}