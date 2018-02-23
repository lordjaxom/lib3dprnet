#include <iostream>

#include <boost/asio/io_context.hpp>
#include <core/filesystem.hpp>
#include <repetier/types.hpp>
#include <repetier/socket.hpp>

using namespace std;
using namespace prnet;

namespace asio = boost::asio;

int main( int argc, char const* const argv[] )
{
    if ( argc != 4 ) {
        std::cerr << "Usage: " << argv[ 0 ] << " <host> <port> <apikey>";
        return 1;
    }

    asio::io_context context;

    rep::settings settings { argv[ 1 ], argv[ 2 ], argv[ 3 ] };

    std::cout << "Connecting to " << settings.host() << ":" << settings.port() << std::endl;

    rep::socket socket( context );
    socket.connect( settings, []( auto ec ) {
        std::cout << "Result: " << ec.message() << std::endl;
    } );

    context.run();

}

