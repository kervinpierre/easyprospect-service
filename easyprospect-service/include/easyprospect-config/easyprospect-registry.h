#pragma once
#include <set>
#include <vector>

namespace easyprospect
{
namespace service
{
    namespace config
    {
        class easyprospect_registry
        {
        private:
            // Listen ports
            std::vector<int> ports_;

        public:
	        std::vector<int> get_ports() const { return ports_; }
	        void set_ports(std::vector<int> val) { ports_ = val; }
        };
    }

} // namespace service
} // namespace easyprospect