#pragma once
#include <set>
#include <vector>

namespace easyprospect
{
namespace service
{
    namespace config
    {
        class ep_worker_client_state
        {
            std::vector<int> port_;
            std::vector<int> pid_;

        public:
            std::vector<int> &get_pid()
            {
                return pid_;
            }
            void set_pid(std::vector<int> val)
            {
                pid_ = val;
            }
            std::vector<int> &get_port()
            {
                return port_;
            }
            void set_port(std::vector<int> val)
            {
                port_ = val;
            }
        };

        class easyprospect_registry
        {
        private:
            // Listen ports
            std::vector<int> port_;
            std::vector<int> pid_;

            // Control server uses these
            std::vector<ep_worker_client_state> worker_clients_;

        public:
	        std::vector<int> get_ports() const { return port_; }
	        void set_ports(std::vector<int> val) { port_ = val; }
            std::vector<int> get_pids() const
            {
                return pid_;
            }
            void set_pids(std::vector<int> val)
            {
                pid_ = val;
            }
            std::vector<ep_worker_client_state> &get_worker_clients()
            {
                return worker_clients_;
            }

            void set_worker_clients(std::vector<ep_worker_client_state> val)
            {
                worker_clients_ = val;
            }
        };
    }

} // namespace service
} // namespace easyprospect