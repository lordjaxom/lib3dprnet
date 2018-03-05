#include <iostream>

#include <boost/asio/io_context.hpp>

#include <core/filesystem.hpp>
#include <core/logging.hpp>
#include <repetier/request.hpp>
#include <repetier/client.hpp>
#include <repetier/types.hpp>

using namespace std;
using namespace prnet;

namespace asio = boost::asio;

int main( int argc, char const* const argv[] )
{
    logger::threshold( logger::Debug );

    if ( argc != 4 ) {
        cerr << "Usage: " << argv[ 0 ] << " <host> <port> <apikey>";
        return 1;
    }

    asio::io_context context;

    rep::settings settings { argv[ 1 ], argv[ 2 ], argv[ 3 ] };

    cout << "Connecting to " << settings.host() << ":" << settings.port() << endl;

    rep::client socket( context, []( auto ec ) {
        cout << "Connect or login failed: " << ec.message() << endl;
    } );

    socket.connect( settings, [&] {
        cout << "Successfully logged in" << endl;

        rep::request request { "ping" };
        request.add_handler( [&]( auto ) {
            cout << "Received pong" << endl;
            socket.close();
        } );
        socket.send( move( request ) );
    } );

    context.run();
}

