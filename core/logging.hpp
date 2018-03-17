#ifndef LIUB3DPRNET_CORE_LOGGING_HPP
#define LIUB3DPRNET_CORE_LOGGING_HPP

#include <cstddef>
#include <memory>
#include <mutex>
#include <ostream>
#include <string>
#include <utility>

#include "config.hpp"

namespace prnet {

namespace detail {

std::ostream& PRNET_DLL logTimestamp( std::ostream &os );
std::ostream& PRNET_DLL logPid( std::ostream &os );

inline void logWrite( std::ostream &os )
{
	os << std::endl;
}

template< typename Arg0, typename ...Args >
void logWrite( std::ostream& os, Arg0&& arg0, Args&&... args )
{
	os << std::forward< Arg0 >( arg0 );
    logWrite( os, std::forward<Args>( args )... );
}

template< typename ...Args >
void logMessage( std::ostream& os, std::string const &tag, char const *level, Args &&... args )
{
    logWrite( os, logTimestamp, " [", logPid, "] [", tag, "] [", level, "] ", std::forward< Args >( args )... );
}

} // namespace detail

class PRNET_DLL Logger
{
public:
	struct PRNET_DLL Level
	{
        static Level const debug;
        static Level const info;
        static Level const warning;
        static Level const error;

		char const* name;
		unsigned level;
	};

private:
	using Lock = std::lock_guard< std::recursive_mutex >;

	static constexpr std::size_t tagLength = 15;

	static bool is( Level const& level );

	static Level const* level_;
	static std::shared_ptr< std::ostream > output_;
	static std::recursive_mutex mutex_;

public:
	static void threshold( Level const& level );
	static void output( std::ostream& output );
	static void output( char const* output );

	explicit Logger( std::string tag );
	Logger( Logger const& ) = delete;

	template< typename ...Args >
	void debug( Args&&... args )
	{
		log( Level::debug, std::forward< Args >( args )... );
	}

	template< typename ...Args >
	void info( Args&&... args )
	{
		log( Level::info, std::forward< Args >( args )... );
	}

	template< typename ...Args >
	void warning( Args&&... args )
	{
		log( Level::warning, std::forward< Args >( args )... );
	}

	template< typename ...Args >
	void error( Args&&... args )
	{
		log( Level::error, std::forward< Args >( args )... );
	}

private:
	template< typename ...Args >
	void log( Level const& level, Args&&... args )
	{
		if ( is( level ) ) {
            Lock lock( mutex_ );
			detail::logMessage( *output_, tag_, level.name, std::forward< Args >( args )... );
		}
	}

	std::string tag_;
};

} // namespace prnet

#endif // LIUB3DPRNET_CORE_LOGGING_HPP
