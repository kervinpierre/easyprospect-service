#include "libEpWeb.h"

auto easyprospect::web::server::EpHTTPServer::make_router(
    task_queue_t& queue,
    request_processor_t& processor)
{
    auto router = std::make_unique<router_t>();

    //
    // Every handler just delegates the actual work to `processor`.
    //

    router->http_get("/all/v1/pets",
        [&queue, &processor](const auto& req, const auto&)
        {
            queue.push(
                task_t{
                    [req, &processor]
{
processor.on_get_all_pets(req);
}
                });
            return restinio::request_accepted();
        });

    router->http_post("/all/v1/pets",
        [&queue, &processor](const auto& req, const auto&)
        {
            queue.push(
                task_t{
                    [req, &processor]
{
processor.on_create_new_pet(req);
}
                });
            return restinio::request_accepted();
        });

    router->http_get(R"--(/all/v1/pets/:id(\d+))--",
        [&queue, &processor](const auto& req, const auto& params)
        {
            const auto id = restinio::cast_to<pet_id_t>(params["id"]);
            queue.push(
                task_t{
                    [req, &processor, id]
{
processor.on_get_specific_pet(req, id);
}
                });
            return restinio::request_accepted();
        });

    router->add_handler(
        restinio::http_method_patch(),
        R"--(/all/v1/pets/:id(\d+))--",
        [&queue, &processor](const auto& req, const auto& params)
        {
            const auto id = restinio::cast_to<pet_id_t>(params["id"]);
            queue.push(
                task_t{
                    [req, &processor, id]
{
processor.on_patch_specific_pet(req, id);
}
                });
            return restinio::request_accepted();
        });

    router->http_delete(R"--(/all/v1/pets/:id(\d+))--",
        [&queue, &processor](const auto& req, const auto& params)
        {
            const auto id = restinio::cast_to<pet_id_t>(params["id"]);
            queue.push(
                task_t{
                    [req, &processor, id]
{
processor.on_delete_specific_pet(req, id);
}
                });
            return restinio::request_accepted();
        });

    return router;
}

void easyprospect::web::server::EpHTTPServer::worker_thread_func(task_queue_t& queue)
{
    for (;;)
    {
        // Try to extract next message to process.
        task_t msg;
        const auto pop_result = queue.pop(msg);
        if (pop_result_t::queue_closed == pop_result)
            break;

        // Extracted task should be executed.
        // NOTE: because this is just example we don't handle
        // exceptions from the task.
        // In production code there should be try-catch blocks with
        // some reaction to an exception: logging of the exception and
        // maybe the correct shutdown of the server.
        msg.m_task();
    }
}

void easyprospect::web::server::EpHTTPServer::run_application()
{
    db_layer_t db{ "pets.db3" };
    request_processor_t processor{ db };

    task_queue_t queue;
    my_thread_pool_t worker_threads_pool{
            3,
            my_shutdowner_t{queue},
            worker_thread_func, std::ref(queue)
    };

    // Default traits are used as a base because they are thread-safe.
    struct my_traits_t : public restinio::default_traits_t
    {
        using request_handler_t = router_t;
    };

    restinio::run(
        restinio::on_this_thread<my_traits_t>()
        .port(8080)
        .address("localhost")
        .request_handler(make_router(queue, processor))
        .cleanup_func([&worker_threads_pool]
            {
                worker_threads_pool.stop();
            }));
}
