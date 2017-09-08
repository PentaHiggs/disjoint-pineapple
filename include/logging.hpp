#ifndef LOGGING_HPP_
#define LOGGING_HPP_

#include<boost/log/trivial.hpp> /*< Severity level and severity logger get imported from here */
#include<boost/log/utility/manipulators/add_value.hpp> /*< Needed for EZ_LOG definition */
#include<iostream>

#define EZ_LOG(log_, sv) BOOST_LOG_SEV( log_, sv) \
	<< boost::log::add_value("Line", __LINE__)     \
	<< boost::log::add_value("File", __FILE__)      \
	<< boost::log::add_value("Function", BOOST_CURRENT_FUNCTION)

namespace loglib {
	/**
	 * Initializes the logging subsystems.  Should only be run once at code entry point
	 */
	void init();

	/**
	 * Importing severity_level enum to namespace for use with logger
	 */
	using boost::log::trivial::severity_level;
	using boost::log::trivial::trace;
	using boost::log::trivial::debug;
	using boost::log::trivial::info;
	using boost::log::trivial::warning;
	using boost::log::trivial::error;
	using boost::log::trivial::fatal;
	/** 
	 * Only logging object I'll need, can use as "loglib::logger"
	 */
	typedef boost::log::sources::severity_logger<severity_level> logger;
}

#endif //LOGGING_HPP_
