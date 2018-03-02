#include <cstring>
#include <ctime>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>

#if !defined( WIN32 )
#   include <sys/types.h>
#   include <unistd.h>
#endif

#include "logging.hpp"

using namespace std;

namespace prnet {

namespace detail {

ostream& log_timestamp( ostream &os )
{
	auto timestamp { chrono::high_resolution_clock::now().time_since_epoch() };
	auto seconds { chrono::duration_cast< chrono::seconds >( timestamp ) };
	auto micros { chrono::duration_cast< chrono::microseconds >( timestamp - seconds ) };
	time_t tt { seconds.count() };
	tm* tm { localtime( &tt ) };

	return os
			<< setw( 4 ) << setfill( '0' ) << ( tm->tm_year + 1900 ) << "/"
			<< setw( 2 ) << setfill( '0' ) << ( tm->tm_mon + 1 ) << "/"
			<< setw( 2 ) << setfill( '0' ) << tm->tm_mday << " "
			<< setw( 2 ) << setfill( '0' ) << tm->tm_hour << ":"
			<< setw( 2 ) << setfill( '0' ) << tm->tm_min << ":"
			<< setw( 2 ) << setfill( '0' ) << tm->tm_sec << "."
			<< setw( 6 ) << setfill( '0' ) << micros.count();
}

ostream& log_pid( ostream& os )
{
	return os << setw( 5 ) << setfill( ' ' ) << getpid();
}

template< size_t L >
string build_tag( string&& tag )
{
	if ( tag.length() == L ) {
		return move( tag );
	}

	string result;
	result.reserve( L );
	if ( tag.length() < L ) {
		result.append( ( L - tag.length() ) / 2, ' ' );
		result.append( tag );
		result.append( ( L - tag.length() ) - ( L - tag.length() ) / 2, ' ' );
	}
	else {
		result.append( "..." );
		result.append( tag.substr( tag.length() - L + 3, L - 3 ) );
	}
	return result;
}

} // namespace detail

logger::level const logger::Debug   { "DEBUG", 3 };
logger::level const logger::Info    { "INFO ", 2 };
logger::level const logger::Warning { "WARN ", 1 };
logger::level const logger::Error   { "ERROR", 0 };

logger::level const* logger::level_ = &logger::Debug;
shared_ptr< ostream > logger::output_( &cerr, []( ostream const* ) {} );

bool logger::is( level const& level )
{
	return level_->level >= level.level;
}

void logger::threshold( level const& level )
{
	level_ = &level;
}

void logger::output( ostream& output )
{
	output_.reset( &output, []( ostream const* ) {} );
}

void logger::output( char const* output )
{
	output_.reset( new ofstream( output, ios::out | ios::app ) );
}

logger::logger( string tag )
	: tag_( detail::build_tag< tagLength >( move( tag ) ) )
{
}

} // namespace prnet
