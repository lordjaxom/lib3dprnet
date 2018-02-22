#include <iostream>

#include <boost/asio/io_context.hpp>
#include <core/filesystem.hpp>
#include <repetier/types.hpp>
#include <repetier/upload.hpp>

using namespace std;
using namespace prnet;

namespace asio = boost::asio;

int main( int argc, char const* const argv[] )
{
    if ( argc != 7 ) {
        std::cerr << "Usage: " << argv[ 0 ] << " <host> <port> <apikey> <printer> <model> <filename>";
        return 1;
    }

    asio::io_context context;

    rep::settings settings { argv[ 1 ], argv[ 2 ], argv[ 3 ] };
    rep::model_ident model { argv[ 4 ], "", argv[ 5 ] };
    filesystem::path path { argv[ 6 ] };

    std::cout << "Uploading " << path << " to " << settings.host() << ":" << settings.port() << std::endl;

    rep::upload_model( context, settings, model, path, []( auto ec ) {
        std::cout << "Result: " << ec.message() << std::endl;
    } );

    context.run();
}