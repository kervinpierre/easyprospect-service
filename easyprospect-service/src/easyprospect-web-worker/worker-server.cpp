#include <easyprospect-web-worker/worker-server.h>
#include <easyprospect-web-worker/worker-server-app.h>

namespace easyprospect
{
namespace service
{
    namespace web_worker
    {

        template <class T, class... Args>
        void insert(channel_list& list, Args&&... args)
        {
            list.insert(boost::make_shared<T>(std::forward<Args>(args)...));
        }

        std::unique_ptr<channel_list_impl> make_channel_list(web_worker::application_impl& srv)
        {
            return boost::make_unique<channel_list_impl>(srv);
        }

        void make_room(channel_list& list, boost::beast::string_view name)
        {
            easyprospect::service::web_worker::insert<room_impl>(list, name, list);
        }

        void make_blackjack_service(web_worker::application_impl& srv)
        {
            srv.insert(boost::make_unique<blackjack_service>(srv));
        }

        void make_system_channel(application_impl& srv)
        {
            web_worker::insert<system_channel>(srv.channel_list(), srv);
        }

    } // namespace web_worker
} // namespace service
} // namespace easyprospect