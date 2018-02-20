#include <boost/asio/buffers_iterator.hpp>
#include <boost/asio/connect.hpp>
#include <nlohmann/json.hpp>

#include "repetier_action.hpp"
#include "repetier_socket.hpp"

using namespace std;

namespace websocket = boost::beast::websocket;

namespace rep {

    RepetierSocket::RepetierSocket( boost::asio::io_context& context, string host, string port, string apikey ):
            resolver_( context ),
            websocket_( context ),
            host_( move( host ) ),
            port_( move( port ) ),
            apikey_( move( apikey ) )
    {
        start();
    }

    RepetierSocket::~RepetierSocket() = default;

    bool RepetierSocket::check( char const* where, boost::system::error_code const& ec )
    {
        if ( !ec ) {
            return true;
        }

        std::cerr << "ERROR: " << ec.message() << " at " << where << "\n";

        close( true );
        return false;
    }

    void RepetierSocket::send( std::unique_ptr< RepetierAction >&& action )
    {
        std::cerr << "DEBUG: <<< " << action->request() << "\n";

        auto callbackId = action->callbackId();
        auto emplaced = pendingActions_.emplace( callbackId, std::move( action ) );

        websocket_.async_write( boost::asio::buffer( emplaced.first->second->request() ),
                                [this, callbackId]( auto const& ec, auto written ) {
                                    if ( !this->check( "write", ec ) ) {
                                        pendingActions_.erase( callbackId );
                                        return;
                                    }
                                } );
    }

    void RepetierSocket::close( bool force )
    {
        ready_ = false;
        websocket_.async_close( force ? websocket::close_code::abnormal : websocket::close_code::normal,
                                [this, force]( auto const& ec ) {
                                    if ( ec ) {
                                        std::cerr << "ERROR: " << ec.message() << " during close\n";
                                    }
                                    if ( force ) {
                                        this->start();
                                    }
                                } );
    }

    void RepetierSocket::start()
    {
        cerr << "INFO: Connecting to " << host_ << ":" << port_ << "\n";

        resolver_.async_resolve( host_, port_, [this]( auto const& ec, auto const& resolved ) {
            if ( !this->check( "resolve", ec ) ) {
                return;
            }
            this->connect( resolved );
        } );
    }

    void RepetierSocket::connect( boost::asio::ip::tcp::resolver::results_type const& resolved )
    {
        cerr << "DEBUG: Connecting...\n";

        boost::asio::async_connect( websocket_.next_layer(), resolved.begin(), resolved.end(),
                                    [this]( auto const& ec, auto const& it ) {
                                        if ( !this->check( "connect", ec ) ) {
                                            return;
                                        }
                                        this->handshake();
                                    } );
    }

    void RepetierSocket::handshake()
    {
        cerr << "DEBUG: Handshake\n";

        websocket_.async_handshake( host_, "/socket", [this]( auto const& ec ) {
            if ( !this->check( "handshake", ec ) ) {
                return;
            }
            this->read();
            this->login();
        } );
    }

    void RepetierSocket::read()
    {
        cerr << "DEBUG: Read\n";

        websocket_.async_read( buffer_, [this]( auto const& ec, auto length ) {
            if ( !this->check( "read", ec ) ) {
                return;
            }

            string buffer( boost::asio::buffers_begin( buffer_.data() ), boost::asio::buffers_end( buffer_.data() ) );

            std::cerr << "DEBUG: >>> " << buffer << "\n";
            std::cerr << "DEBUG: length = " << length << ", bufferlength = " << buffer.size() << ", size = " << buffer_.size() << "\n";

            auto response = nlohmann::json::parse( buffer );
            buffer_.consume( buffer.size() );

            auto callbackId = response[ "callback_id" ];
            if ( callbackId != -1 ) {
                this->readCallback( callbackId, response );
                std::cerr << "DEBUG: read callback " << callbackId << "\n";
            } else if ( response[ "eventList" ] ) {
                std::cerr << "DEBUG: read events\n";
            }

            this->read();
        } );
    }

    void RepetierSocket::readCallback( size_t callbackId, nlohmann::json const& data )
    {
        auto it = pendingActions_.find( callbackId );
        if ( it == pendingActions_.end() ) {
            cerr << "ERROR: spurious callback " << callbackId << " received\n";
            return;
        }

        boost::system::error_code ec;
        it->second->handle( data[ "data" ], ec );
        if ( ec ) {
            cerr << "ERROR: invalid response to callback " << callbackId << ": " << ec.message() << "\n";
        }
    }

    void RepetierSocket::login()
    {
        cerr << "DEBUG: Login\n";

        RepetierActionBuilder( "login" )
                .arg( "apikey", apikey_ )
                .handler( RepetierAction::checkOk() )
                .handler( [this]( auto const& data, auto& ec ) {
                    cerr << "DEBUG: handler2\n";
                    if ( ec ) {
                        std::cerr << "ERROR: Login failed, closing connection\n";
                        this->close( true );
                    }
                    cerr << "DEBUG: Login successful\n";
                    ready_ = true;
                } )
                .send( *this );
    }

} // namespace rep