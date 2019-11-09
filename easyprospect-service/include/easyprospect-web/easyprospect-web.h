#pragma once
#include <restinio/all.hpp>

#include "multithreading.h"
#include "request_processor.h"
#include "framework.h"

namespace easyprospect
{
    namespace web
    {
        namespace server
        {
            // Type of message for pushing tasks to a pool of worker threads.
            struct task_t
            {
                std::function<void()> m_task;

                task_t() = default;

                template<typename F>
                task_t(F&& task) : m_task{ std::forward<F>(task) } {}
            };

            // Type of message queue of task_t objects.
            using task_queue_t = message_queue_t<task_t>;

            // Type of shutdowner to be used with thread-pool.
            // Closes the specified task-queue. This gives a signal to
            // worker threads to finish their work.
            class my_shutdowner_t
            {
                task_queue_t& m_queue;
            public:
                my_shutdowner_t(task_queue_t& queue) : m_queue{ queue } {}

                void operator()() noexcept
                {
                    m_queue.close();
                }
            };

            using my_thread_pool_t = thread_pool_t<my_shutdowner_t>;

            class EpHTTPServer
            {
            public:

                // Short alias for express-like router.
                using router_t = restinio::router::express_router_t<>;

                static auto make_router(
                    task_queue_t& queue,
                    request_processor_t& processor);

                static void worker_thread_func(task_queue_t& queue);

                static void run_application();
            };
        }
    }
}