#include <iostream>
#include <easyprospect-web.h>

int main(int argc, char *argv[])
{
    try
    {
        auto server = std::make_unique<easyprospect::web::server::EpHTTPServer>();

        server->run_application();
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