#include <iostream>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <repetier/service.hpp>

using namespace std;
using namespace prnet;

using tcp = boost::asio::ip::tcp;

namespace asio = boost::asio;
namespace http = boost::beast::http;

template< typename Callback >
auto fhemCommunicate( asio::io_context& context, asio::yield_context yield, Callback&& cb )
{
    cerr << "fhemCommunicate: resolve\n";

    tcp::resolver resolver { context };
    auto resolved { resolver.async_resolve( "speedstar", "8083", yield ) };

    cerr << "fhemCommunicate: connect\n";
    tcp::socket socket { context };
    asio::async_connect( socket, resolved, yield );

    return cb( socket );
}

auto fhemGetToken( asio::io_context& context, asio::yield_context yield )
{
    return fhemCommunicate( context, yield, [&]( auto& socket ) {
        cerr << "fhemGetToken: XHR request\n";
        http::request< http::empty_body > request { http::verb::get, "/fhem?xhr=1", 11 };
        request.set( http::field::host, "speedstar" );
        request.set( http::field::user_agent, BOOST_BEAST_VERSION_STRING );
        http::async_write( socket, request, yield );

        cerr << "fhemGetToken: XHR response\n";
        boost::beast::multi_buffer buffer;
        http::response< http::dynamic_body > response;
        http::async_read( socket, buffer, response, yield );

        cerr << "fhemGetToken: status " << response.result_int() << ", token " << response[ "X-FHEM-csrfToken" ] << "\n";

        return string( response[ "X-FHEM-csrfToken" ] );
    } );
}

void fhemSetPrinterState( asio::io_context& context, asio::yield_context yield, string const& printer, string const& state )
{
    fhemCommunicate( context, yield, [&, token { fhemGetToken( context, yield ) }]( auto& socket ) {
        auto target { "/fhem?fwcsrf=" + token + "&cmd=setreading%20MakerPI%20printerState_" + printer + "%20" + state };

        cerr << "fhemSetPrinterState: request " << target << "\n";

        http::request< http::empty_body > request { http::verb::get, target, 11 };
        request.set( http::field::host, "speedstar" );
        request.set( http::field::user_agent, BOOST_BEAST_VERSION_STRING );
        http::async_write( socket, request, yield );

        cerr << "fhemSetPrinterState: response\n";
        boost::beast::multi_buffer buffer;
        http::response< http::dynamic_body > response;
        http::async_read( socket, buffer, response, yield );

        cerr << "fhemSetPrinterState: status " << response.result_int() << "\n";
    } );
}

void fhemSetPrinterState( asio::io_context& context, string printer, string state )
{
    asio::spawn( context, [&, printer { move( printer ) }, state { move( state ) }]( auto yield ) {
        fhemSetPrinterState( context, yield, printer, state );
    } );
}


int main( int argc, char const* const argv[] )
{
    if ( argc != 4 ) {
        cerr << "Usage: " << argv[ 0 ] << " <host> <port> <apikey>";
        return 1;
    }

    asio::io_context context;
    rep::settings settings { argv[ 1 ], argv[ 2 ], argv[ 3 ] };
    rep::service service { context, settings };
    service.job_started.connect( [&]( auto printer ) { fhemSetPrinterState( context, move( printer ), "printing" ); } );
    service.job_finished.connect( [&]( auto printer ) { fhemSetPrinterState( context, move( printer ), "idle" ); } );
    service.job_killed.connect( [&]( auto printer ) { fhemSetPrinterState( context, move( printer ), "idle" ); } );

    context.run();
}

