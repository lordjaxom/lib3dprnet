#include "repetier_socket.hpp"

using namespace std;

int main()
{
    boost::asio::io_context io_context;

    rep::RepetierSocket socket( io_context, "makerpi", "3344", "45607d2e-1bc4-4c32-8f10-0fc06d82bdcf" );

    io_context.run();
}