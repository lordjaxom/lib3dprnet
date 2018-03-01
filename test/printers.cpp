#include <iostream>

#include <boost/asio/io_context.hpp>
#include <repetier/service.hpp>

using namespace std;
using namespace prnet;

namespace asio = boost::asio;

int main( int argc, char const* const argv[] )
{
    if ( argc != 4 ) {
        cerr << "Usage: " << argv[ 0 ] << " <host> <port> <apikey>";
        return 1;
    }

    asio::io_context context;

    rep::settings settings { argv[ 1 ], argv[ 2 ], argv[ 3 ] };

    cout << "Connecting to " << settings.host() << ":" << settings.port() << endl;

    rep::service service { context, settings };
    service.list_printer( [&]( auto printers ) {
        for ( auto const& printer : printers ) {
            cout << "PRINTER: " << printer.name() << " " << printer.slug() << endl;
            service.list_groups( printer.slug(), [=]( auto groups ) {
                for ( auto const& group : groups ) {
                    cout << "GROUP FOR PRINTER: " << printer.name() << " - " << group.name() << endl;
                }
            } );
        }
    } );

    context.run();
}

