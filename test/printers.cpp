#include <iostream>

#include <boost/asio/io_context.hpp>

#include <core/logging.hpp>
#include <repetier/service.hpp>

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

    rep::Endpoint settings { argv[ 1 ], argv[ 2 ], argv[ 3 ] };

    cout << "Connecting to " << settings.host() << ":" << settings.port() << endl;

    rep::Service service { context, settings };
/*    Service.list_printers( [&]( auto printers ) {
        for ( auto const& Printer : printers ) {
            cout << "PRINTER: " << Printer.name() << " " << Printer.slug() << endl;
            Service.list_groups( Printer.slug(), [=]( auto groups ) {
                for ( auto const& ModelGroup : groups ) {
                    cout << "GROUP FOR PRINTER: " << Printer.name() << " - " << ModelGroup.name() << endl;
                }
            } );
        }
    } );*/

    context.run();
}

