#include <cstdint>
#include <iostream>
#include <initializer_list>
#include <functional>
#include <string>
#include <utility>

#include <boost/asio/connect.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http/dynamic_body.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/version.hpp>
#include <boost/optional/optional.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

struct multipart_body
{
    class part;
    class value_type;
    class writer;
};

class multipart_body::part
{
    friend class writer;

public:
    part( std::string name, std::string value )
            : name_( std::move( name ) )
            , value_( std::move( value ) ) {}

    part( std::string name, std::string filename, std::string value )
            : name_( std::move( name ) )
            , filename_( std::move( filename ) )
            , value_( std::move( value ) ) {}

private:
    std::string name_;
    boost::optional< std::string > filename_;
    std::string value_;
};

class multipart_body::value_type
{
    friend class writer;

public:
    value_type() {}

    template< typename InputIt >
    value_type( InputIt first, InputIt last )
            : parts_ { first, last } {}

    value_type( std::initializer_list< part > init )
            : parts_ { init } {}

    void push_back( part const& part ) { parts_.push_back( part ); }
    void push_back( part&& part ) { parts_.push_back( std::move( part ) ); }

    template< typename ...Args >
    void emplace_back( Args&&... args ) { parts_.emplace_back( std::forward< Args >( args )... ); }

private:
    std::string boundary_ { boost::uuids::to_string( boost::uuids::random_generator()() ) };
    std::vector< part > parts_;
};

class multipart_body::writer
{
public:
    using const_buffers_type = boost::asio::streambuf::const_buffers_type;

    writer( boost::beast::http::request< multipart_body >& request )
            : request_ { request } {}

    void init( boost::beast::error_code& ec )
    {
        ec.assign( 0, ec.category() );
        request_.set( boost::beast::http::field::content_type,
                      "multipart/form-data; boundary=" + request_.body().boundary_ );
        partsIt_ = request_.body().parts_.cbegin();
    }

    boost::optional< std::pair< const_buffers_type, bool > > get( boost::beast::error_code& ec )
    {
        buffer_.consume( buffer_.size() );
        std::ostream os( &buffer_ );

        bool last = partsIt_ == request_.body().parts_.cend();
        os << "--" << request_.body().boundary_;
        if ( last ) {
            os << "--\015\012";
        } else {
            os << "\015\012Content-Disposition: form-data; name=\"" << partsIt_->name_ << "\"";
            if ( partsIt_->filename_ ) {
                os << "; filename=\"" << *partsIt_->filename_ << "\"";
            }
            os << "\015\012\015\012" << partsIt_->value_ << "\015\012";
            ++partsIt_;
        }
        return {{ buffer_.data(), !last }};
    }

private:
    boost::beast::http::request< multipart_body >& request_;
    std::vector< part >::const_iterator partsIt_;
    boost::asio::streambuf buffer_;
};

namespace boost::beast::http {
    template<> struct is_body_writer< multipart_body > : std::true_type {};
}

class uploader
{
public:
    uploader( boost::asio::io_context& context ):
            context_ { context }
    {
    }

    void upload( std::function< void ( boost::system::error_code ) > handler )
    {
        boost::asio::spawn( context_, [this, handler { std::move( handler ) }]( auto yield ) {
            this->do_upload( std::move( handler ), yield );
        } );
    }

private:
    void do_upload( std::function< void ( boost::system::error_code ) > handler, boost::asio::yield_context yield )
    {
        boost::system::error_code ec;
        try {
            boost::asio::ip::tcp::resolver resolver { context_ };
            auto resolved = resolver.async_resolve( "makerpi", "3344", yield );

            boost::asio::ip::tcp::socket socket { context_ };
            boost::asio::async_connect( socket, resolved, yield );

            boost::beast::http::request< multipart_body > request {
                    boost::beast::http::verb::post, "/printer/model/Replicator", 11 };
            request.set( boost::beast::http::field::host, "makerpi" );
            request.set( boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING );
            request.set( "x-api-key", "45607d2e-1bc4-4c32-8f10-0fc06d82bdcf" );
            request.body().emplace_back( "a", "upload" );
            request.body().emplace_back( "name", "modelName" );
            request.body().emplace_back( "group", "groupName" );
            request.body().emplace_back( "filename", "test.gcode", "wulle\nwulle\nwulle\nwutz\n" );

            if ( ec ) {
                goto error;
            }
            boost::beast::http::async_write( socket, request, yield );

            boost::beast::multi_buffer buffer;
            boost::beast::http::response<boost::beast::http::dynamic_body> response;
            boost::beast::http::async_read( socket, buffer, response, yield );
            if ( response.result() != boost::beast::http::status::ok &&
                    response.result() != boost::beast::http::status::no_content ) {
                ec = { boost::system::errc::invalid_argument, boost::system::generic_category() };
                goto error;
            }

            std::cout << response << "\n";
        } catch ( boost::system::system_error const& e ) {
            std::cout << "caught inside\n";
            ec = e.code();
        }

    error:
        handler( std::move( ec ) );
    }

    boost::asio::io_context& context_;
};

int main() {
    std::cout << "Hello, World!\n";

    boost::asio::io_context context;
    uploader u { context };
    u.upload( []( auto ec ) {
        std::cout << "Done: " << ec.message() << "\n";
    } );

    context.run();
}