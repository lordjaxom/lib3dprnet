#ifndef LIUB3DPRNET_CORE_LOGGING_HPP
#define LIUB3DPRNET_CORE_LOGGING_HPP

#include <cstddef>
#include <memory>
#include <ostream>
#include <string>
#include <utility>

#include "config.hpp"

namespace prnet {

namespace detail {

std::ostream& PRNET_DLL log_timestamp( std::ostream &os );
std::ostream& PRNET_DLL log_pid( std::ostream &os );

inline void log_write( std::ostream& os )
{
	os << std::endl;
}

template< typename Arg0, typename ...Args >
void log_write( std::ostream& os, Arg0&& arg0, Args&&... args )
{
	os << std::forward< Arg0 >( arg0 );
	log_write( os, std::forward< Args >( args )... );
}

template< typename ...Args >
void log_message( std::ostream& os, std::string const &tag, char const *level, Args &&... args )
{
	log_write( os, log_timestamp, " [", log_pid, "] [", tag, "] [", level, "] ", std::forward< Args >( args )... );
}

} // namespace detail

class PRNET_DLL logger
{
	struct level
	{
		char const* name;
		unsigned level;
	};

	static constexpr std::size_t tagLength = 15;

	static bool is( level const& level );

	static level const* level_;
	static std::shared_ptr< std::ostream > output_;

public:
	static level const Debug;
	static level const Info;
	static level const Warning;
	static level const Error;

	static void threshold( level const& level );
	static void output( std::ostream& output );
	static void output( char const* output );

	explicit logger( std::string tag );
	logger( logger const& ) = delete;

	template< typename ...Args >
	void debug( Args&&... args )
	{
		log( Debug, std::forward< Args >( args )... );
	}

	template< typename ...Args >
	void info( Args&&... args )
	{
		log( Info, std::forward< Args >( args )... );
	}

	template< typename ...Args >
	void warning( Args&&... args )
	{
		log( Warning, std::forward< Args >( args )... );
	}

	template< typename ...Args >
	void error( Args&&... args )
	{
		log( Error, std::forward< Args >( args )... );
	}

private:
	template< typename ...Args >
	void log( level const& level, Args&&... args )
	{
		if ( is( level ) ) {
			detail::log_message( *output_, tag_, level.name, std::forward< Args >( args )... );
		}
	}

	std::string tag_;
};

} // namespace prnet

#endif // LIUB3DPRNET_CORE_LOGGING_HPP
