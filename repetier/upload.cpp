#include <ostream>
#include <unordered_map>
#include <utility>

#include <boost/asio/connect.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/file.hpp>
#include <boost/beast/core/multi_buffer.hpp>
#include <boost/beast/http/dynamic_body.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/version.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <utf8.h>

#include "core/error.hpp"
#include "core/logging.hpp"
#include "core/string_view.hpp"
#include "types.hpp"
#include "upload.hpp"

using namespace std;

namespace asio = boost::asio;
namespace http = boost::beast::http;
namespace uuids = boost::uuids;

using tcp = asio::ip::tcp;

namespace prnet {
namespace rep {

static logger logger( "rep::Upload" );

namespace detail {

/**
 * struct upload_body
 */

struct upload_body
{
    class value_type;
    class writer;
};


/**
 * class upload_body::value_type
 */

class upload_body::value_type
{
    friend class writer;

public:
    using fields_type = unordered_map< string_view, string_view >;

    void set( string_view name, string_view value )
    {
        fields_.emplace( name, value );
    }

    void set( filesystem::path const& path )
    {
        path_ = &path;
    }

private:
    fields_type fields_;
    filesystem::path const* path_ {};
};


/**
 * class upload_body::writer
 */

class upload_body::writer
{
    static constexpr size_t blockSize { 8192 };

public:
    using const_buffers_type = asio::streambuf::const_buffers_type;

    template< bool isRequest, typename Fields >
    writer( http::message< isRequest, upload_body, Fields >& request )
            : body_ { request.body() }
    {
        request.set( http::field::content_type, "multipart/form-data; boundary=" + uuids::to_string( boundary_ ) );
    }

    void init( boost::beast::error_code& ec )
    {
        ec.assign( 0, ec.category() );

        field_ = body_.fields_.cbegin();
        if ( body_.path_ ) {
            file_.open( body_.path_->u8string().c_str(), boost::beast::file_mode::read, ec );
        }
    }

    boost::optional< pair< const_buffers_type, bool > > get( boost::beast::error_code& ec )
    {
        buffer_.consume( buffer_.size() );

        bool more { true };
        switch ( state_ ) {
            case 0: {
                if ( field_ != body_.fields_.cend() ) {
                    ostream os { &buffer_ };
                    os << "--" << boundary_ << "\r\n"
                       << "Content-Disposition: form-data; name=\"" << field_->first << "\"\r\n"
                       << "Content-Type: text/plain; charset=utf-8\r\n\r\n";
                    for_each( field_->second.begin(), field_->second.end(),
                              [out = ostream_iterator< uint8_t >( os )]( uint8_t ch ) {
                        utf8::append( static_cast< uint32_t >( ch ), out );
                    } );
                    os << "\r\n";
                    ++field_;
                    break;
                }
                ++state_;
                // fall through
            }

            case 1: {
                ++state_;
                if ( file_.is_open() ) {
                    ostream os { &buffer_ };
                    os << "--" << boundary_ << "\r\n"
                       << "Content-Disposition: form-data; name=\"filename\"; filename=\"" << body_.path_->filename().string() << "\"\r\n"
                       << "Content-Type: application/octet-stream\r\n\r\n";
                    break;
                }
                // fall through
            }

            case 2: {
                if ( file_.is_open() ) {
                    auto buf { buffer_.prepare( blockSize ) };
                    size_t read {};
                    for ( auto it { buf.begin() } ; it != buf.end() ; ++it ) {
                        size_t r { file_.read( it->data(), it->size(), ec ) };
                        if ( r == 0 || ec ) {
                            break;
                        }
                        read += r;
                    }
                    if ( read > 0 ) {
                        buffer_.commit( read );
                        break;
                    }
                }
                ++state_;
                // fall through
            }

            case 3: {
                ostream os { &buffer_ };
                os << "\r\n"
                   << "--" << boundary_ << "--\r\n";
                more = false;
            }
        }

        return {{ buffer_.data(), more }};
    }

private:
    value_type& body_;
    asio::streambuf buffer_;
    size_t state_ {};
    value_type::fields_type::const_iterator field_;
    boost::beast::file file_;
    uuids::uuid boundary_ { uuids::random_generator()() };
};

} // namespace detail


/**
 * function uploadModel
 */

void uploadModel( boost::asio::io_context& context, Endpoint const& settings, model_ident ident,
                  filesystem::path path, UploadHandler handler )
{
    asio::spawn( context, [&context, &settings, ident = move( ident ), path = move( path ), handler = move( handler )]( auto yield ) {
        error_code ec;
        try {
            tcp::resolver resolver { context };
            auto resolved { resolver.async_resolve( settings.host(), settings.port(), yield ) };

            tcp::socket socket { context };
            asio::async_connect( socket, resolved, yield );

            http::request< detail::upload_body > request { http::verb::post, "/printer/model/" + ident.printer(), 11 };
            request.set( http::field::host, settings.host() );
            request.set( http::field::user_agent, BOOST_BEAST_VERSION_STRING );
            request.set( "x-api-key", settings.apikey() );
            request.body().set( "a", "upload" );
            request.body().set( "name", ident.name() );
            request.body().set( "group", ident.group() );
            request.body().set( path );

            http::async_write( socket, request, yield );

            boost::beast::multi_buffer buffer;
            http::response< http::dynamic_body > response;
            http::async_read( socket, buffer, response, yield );
            if ( response.result() != http::status::ok && response.result() != http::status::no_content ) {
                ec = make_error_code( prnet_errc::server_error );
            }
        } catch ( system_error const& e ) {
            logger.error( "error: ", e.code().message() );
            ec = e.code();
        } catch ( boost::beast::system_error const& e ) {
            logger.error( "error: ", e.code().message() );
            ec = e.code();
        }
        handler( ec );
    } );
}

} // namespace rep
} // namespace prnet
