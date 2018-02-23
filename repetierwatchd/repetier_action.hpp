#ifndef REPETIERWATCHD_REPETIER_ACTION_HPP
#define REPETIERWATCHD_REPETIER_ACTION_HPP

#include <cstddef>
#include <string>

#include <boost/system/error_code.hpp>
#include <nlohmann/json.hpp>

namespace rep {

    class RepetierSocket;

    class RepetierAction
    {
    public:
        using Handler = std::function< void ( nlohmann::json const&, boost::system::error_code& ) >;

        static Handler checkOk();

        RepetierAction( std::size_t callbackId, std::string&& request, Handler&& handler );

        std::size_t callbackId() const { return callbackId_; }
        std::string const& request() const { return request_; }

        void handle( nlohmann::json const& data, boost::system::error_code& ec );

    private:
        std::size_t callbackId_;
        std::string request_;
        Handler handler_;
    };

    class RepetierActionBuilder
    {
    public:
        using Handler = RepetierAction::Handler;

        RepetierActionBuilder( std::string action );
        RepetierActionBuilder( std::string action, std::string printer );

        template< typename T >
        RepetierActionBuilder& arg( std::string name, T&& value )
        {
            json_[ "data" ][ name ] = std::forward< T >( value );
            return *this;
        }

        template< typename Handler >
        RepetierActionBuilder& handler( Handler handler )
        {
            if ( !handler_ ) {
                handler_ = std::move( handler );
            } else {
                auto first = std::move( handler_ );
                auto second = std::move( handler );
                handler_ = [first, second]( auto const& data, auto& ec ) {
                    if ( !ec ) { first( data, ec ); }
                    if ( !ec ) { second( data, ec ); }
                };
            }
            return *this;
        }

        void send( RepetierSocket& socket );

    private:
        nlohmann::json json_;
        Handler handler_;
    };

} // namespace rep

#endif //REPETIERWATCHD_REPETIER_ACTION_HPP
