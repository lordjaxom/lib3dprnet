#ifndef REPETIERWATCHD_REPETIER_SOCKET_HPP
#define REPETIERWATCHD_REPETIER_SOCKET_HPP

#include <unordered_map>
#include <memory>
#include <string>
#include <system_error>
#include <utility>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <nlohmann/json_fwd.hpp>

namespace rep {

    class RepetierAction;

    class RepetierSocket
    {
        friend class RepetierActionBuilder;

    public:
        RepetierSocket( boost::asio::io_context& context, std::string host, std::string port, std::string apikey );
        ~RepetierSocket();

    private:
        bool check( char const* where, boost::system::error_code const& ec );
        void send( std::unique_ptr< RepetierAction >&& action );
        void close( bool force );

        void start();
        void connect( boost::asio::ip::tcp::resolver::results_type const& resolved );
        void handshake();
        void login();

        void read();
        void readCallback( std::size_t callbackId, nlohmann::json const& data );

        std::size_t nextCallbackId() { return ++lastCallbackId_; }

        boost::asio::ip::tcp::resolver resolver_;
        boost::beast::websocket::stream< boost::asio::ip::tcp::socket > websocket_;
        std::string host_;
        std::string port_;
        std::string apikey_;
        bool ready_ {};
        boost::beast::multi_buffer buffer_;
        std::size_t lastCallbackId_ = {};
        std::unordered_map< std::size_t, std::unique_ptr< RepetierAction > > pendingActions_;
    };

} // namespace rep

#endif //REPETIERWATCHD_REPETIER_SOCKET_HPP
