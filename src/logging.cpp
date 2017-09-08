// Adapated from https://stackoverflow.com/questions/31154429/boost-log-support-file-name-and-line-number/31160870#31160870

// STD includes
#include <ostream>
#include <fstream>
#include <mutex>

// Boost includes
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
//#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/filesystem.hpp>

// Header include
#include"logging.hpp"

namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace logging_ = boost::log;

static void my_formatter(logging_::record_view const& rec, logging_::formatting_ostream& strm)
{
	// Add timestamp information
	const boost::posix_time::ptime &pt = *logging_::extract< boost::posix_time::ptime >("TimeStamp", rec);	
    strm << pt.date() << " " << std::setw(2) << std::setfill('0') << pt.time_of_day().hours() << ":";
	strm << std::setw(2) << std::setfill('0') << pt.time_of_day().minutes();
	strm << ":" << std::setw(2) << std::setfill('0') << pt.time_of_day().seconds() << " | ";

	// Get the LineID attribute value and put it into the stream
    //strm << logging_::extract< unsigned int >("LineID", rec) << ": ";
    logging_::value_ref< std::string > fullpath = logging_::extract< std::string >("File", rec);
    strm << boost::filesystem::path(fullpath.get()).filename().string();
	
	// Get the line number and put it into stream
	strm << "[";
	strm << std::setw(3);
    strm << logging_::extract< int >("Line", rec) << "]: ";

    // The same for the severity level.
    // The simplified syntax is possible if attribute keywords are used.
    strm << "<" << rec[logging_::trivial::severity] << "> ";
	
    // Finally, put the record message to the stream
    strm << rec[expr::smessage];
}

void loglib::init()
{
	/* static (locally namespaced global) variables
	 * Forgive me father, for I have sinned */
	static bool run_yet(false);
	static std::mutex mut_;

	// TODO: does std::mutex mut_ and the static bool have to be atomic or something to avoid
	// difficult to resolve bugs?
	std::unique_lock<std::mutex> lock(mut_);
	if (run_yet) {
		return;	// Don't need to run it again
	} else {
		run_yet = true;
		lock.unlock();
	}
	// No thread will make it this far besides the first; we are safe here.
    typedef sinks::synchronous_sink< sinks::text_ostream_backend > text_sink;
    boost::shared_ptr< text_sink > sink = boost::make_shared< text_sink >();

    sink->locked_backend()->add_stream(
        boost::make_shared< std::ofstream >("logs/debug.log" , std::ios::out | std::ios::app));

    sink->set_formatter(&my_formatter);

    logging_::core::get()->add_sink(sink);

	boost::log::add_common_attributes();
}
