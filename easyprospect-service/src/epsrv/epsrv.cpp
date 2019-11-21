#include <iostream>
#include <easyprospect-web/easyprospect-web.h>

int main(int argc, char *argv[])
{
    try
    {
      //  auto server = std::make_unique<easyprospect::web::server::EpHTTPServer>();

     //   server->run_application();

            // Check command line arguments.
        if (argc != 5)
        {
            std::cerr <<
                "Usage: advanced-server-flex <address> <port> <doc_root> <threads>\n" <<
                "Example:\n" <<
                "    advanced-server-flex 0.0.0.0 8080 . 1\n";
            return EXIT_FAILURE;
        }
        auto const address = net::ip::make_address(argv[1]);
        auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
        auto const doc_root = std::make_shared<std::string>(argv[3]);
        auto const threads = std::max<int>(1, std::atoi(argv[4]));

        // The io_context is required for all I/O
        net::io_context ioc{ threads };

        // The SSL context is required, and holds certificates
        ssl::context ctx{ ssl::context::tlsv12 };

        // This holds the self-signed certificate used by the server
        load_server_certificate(ctx);

        // Create and launch a listening port
        std::make_shared<listener>(
            ioc,
            ctx,
            tcp::endpoint{ address, port },
            doc_root)->run();

        // Capture SIGINT and SIGTERM to perform a clean shutdown
        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait(
            [&](beast::error_code const&, int)
            {
                // Stop the `io_context`. This will cause `run()`
                // to return immediately, eventually destroying the
                // `io_context` and all of the sockets in it.
                ioc.stop();
            });

        // Run the I/O service on the requested number of threads
        std::vector<std::thread> v;
        v.reserve(threads - 1);
        for (auto i = threads - 1; i > 0; --i)
            v.emplace_back(
                [&ioc]
                {
                    ioc.run();
                });
        ioc.run();

        // (If we get here, it means we got a SIGINT or SIGTERM)

        // Block until all the threads exit
        for (auto& t : v)
            t.join();
    }
    catch (const std::exception & x)
    {
        std::cerr << "Exception caught: " << x.what() << std::endl;
        return 2;
    }
    catch (...)
    {
        std::cerr << "Unknown exception" << std::endl;
        return 3;
    }

    return 0;
}