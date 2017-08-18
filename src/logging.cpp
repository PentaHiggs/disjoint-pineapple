// Adapated from https://stackoverflow.com/questions/31154429/boost-log-support-file-name-and-line-number/31160870#31160870

// STD includes
#include <ostream>
#include <fstream>

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
    strm << pt.date() << " " << pt.time_of_day().hours() << ":" << pt.time_of_day().minutes();
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
    typedef sinks::synchronous_sink< sinks::text_ostream_backend > text_sink;
    boost::shared_ptr< text_sink > sink = boost::make_shared< text_sink >();

    sink->locked_backend()->add_stream(
        boost::make_shared< std::ofstream >("logs/debug.log"));

    sink->set_formatter(&my_formatter);

    logging_::core::get()->add_sink(sink);

	boost::log::add_common_attributes();
}
