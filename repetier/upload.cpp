#include "core/error.hpp"
#include "repetier/upload.hpp"

#include <boost/asio/spawn.hpp>

using namespace std;

namespace prnet {
namespace rep {

void upload_model( boost::asio::io_context& context, settings const& settings, model_ident const& ident,
                   prnet::filesystem::path const& path, std::function< void ( std::error_code ) > handler )
{

    error_code ec;
    try {

    } catch ( system_error const& e ) {
        ec = e.code();
    }

error:

}

} // namespace rep
} // namespace prnet
