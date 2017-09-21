#include <iostream>
#include <thread>
#include <vector>
#include <unordered_map>
#include <memory>
#include <cassert>
#include <sstream>
#include <signal.h>

#include "logging.hpp"
#include "util.hpp"
#include "ocrWrapper.hpp"
#include "api_fetch.hpp"

#include <leptonica/allheaders.h>  // Contains all of the headers for leptonica functionality (class pix defined there
#include <tesseract/renderer.h> // For renderer functionality
#include <string>
using std::string;
#include <tesseract/baseapi.h>// Contains the class definitions for tesseract::TessBaseApi.  Requires using std::string in unichar.h
// Include this file in order to use GlobalDawgCache()
///#include <tesseract/dict.h>
#include <tesseract/publictypes.h> // Include this in order to get access to tesseract settings enums
#include <boost/date_time/gregorian/gregorian.hpp>

using std::cout;
using std::cerr;
using std::endl;
using std::string;

using loglib::debug;

using boost::gregorian::date;
using boost::gregorian::days;
using boost::gregorian::from_simple_string;

// Global variable (gasp) used for orderly winding down main.cpp's main loop
volatile sig_atomic_t SIGINT_RECEIVED = 0;

// Simple struct holding both a string buffer and a name
struct bufferStruct {
	std::unique_ptr<std::string> buffer;
	std::string name;
	bool kill_muncher;

	// Initialize buffer_and_name object, but without flipping the kill_muncher switch
	bufferStruct(std::unique_ptr<std::string> buffer_, std::string name_) :
		buffer(std::move(buffer_)),
		name(name_),
		kill_muncher(false) {};

	bufferStruct(std::unique_ptr<std::string> buffer_, std::string name_, bool kill) :
		buffer(std::move(buffer_)),
		name(name_),
		kill_muncher(kill) {};

};

// Function that runs_ocr on elements from a SharedQueue of buffers.
// Keeps asking for elements from buffer_queue.dequeue(), until it receives
// an empty smart pointer, after which the function terminates.
void buffer_muncher(util::SharedQueue< std::unique_ptr<bufferStruct> > *buffer_queue, int pid) {
	loglib::init();
	loglib::logger lg_;
	util::dict settings = {{"dummy_setting","meow"}};
	OcrWrapper ocr(settings);
	ocr.init();
	// Keep munching buffers until you get a nullptr
	while (true) {
		// Following function will wait on buffer_queue to have bufferStruct elements to munch
		std::unique_ptr<bufferStruct> buffer_struct_ptr = std::move(buffer_queue->dequeue());
		if (!buffer_struct_ptr->kill_muncher) {
			assert(buffer_struct_ptr->buffer->size() > 0);	// Pointer should not be empty.
			ocr.classifyFile(buffer_struct_ptr->buffer.get(), buffer_struct_ptr->name);
			EZ_LOG(lg_, loglib::trace) << "Munched " << buffer_struct_ptr->name;
		} else {	// we kill the muncher.
			EZ_LOG(lg_, loglib::trace) << "Shutdown order received @ muncher " << pid;
			return;
		}
	// buffer_struct object naturally deallocated here, along with its buffer and name
	}
	return;
}

// Parses the input in order to determine start and end dates, expecting input
// of the form YYYY-MM-DD, e.g. 1840-12-29
// If this parsing fails, returns false.  Else, returns true
bool parse_input(date& start_date, date& end_date, std::string& start_date_str,
		std::string& end_date_str, int argn, const char**& argv) {
	if (argn != 3) {
		return false;
	}
	try {
		start_date = from_simple_string(argv[1]);
		end_date = from_simple_string(argv[2]);
	} catch (...) {
		return false;
	}
	if (start_date.is_not_a_date() || end_date.is_not_a_date() ) 
		return false;
	return true;	
}

int main(int argn, const char** argv) {	
	loglib::init();
	loglib::logger lg_;
	
	date start_date;
	date end_date;
	std::string start_date_str;
	std::string end_date_str;

	if (!parse_input(start_date, end_date, start_date_str, end_date_str, argn, argv)) {
		return false;
	}
	
	/***********************  Main Control Variables ************************/
	const int num_ocr_instances = 3;			// Number of OCR instances to run.  Should equal number of cores on machine
	const int buffer_queue_max_size = 2;		// Doesn't need to be very large.  Increase if internet connection is unreliable
	const int num_pages_to_classify = 2;		// How many pages deep to go into every daily issue

	util::SharedQueue< std::unique_ptr<bufferStruct> > buffer_queue(buffer_queue_max_size);
	
	util::dict dictionary;
	std::vector<std::thread> ocr_threads;
	for(int i = 0; i < num_ocr_instances; ++i) {	
		ocr_threads.push_back( std::thread(buffer_muncher, &buffer_queue, i) );
	}

	ApiFetch urlFetcher = ApiFetch();
	urlFetcher.init();
	
	/* Mechanism for interrupting loop using CTRL-C */
	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = [](int s) {SIGINT_RECEIVED=1;}
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);

	std::stringstream strBuilder;
	for(date curr_date = start_date; curr_date <= end_date; curr_date += days(1)) {
		for(int i = 1; i <= num_pages_to_classify; i++) {
			// Incase the program was given a (friendly) order to halt
			if(SIGINT_RECEIVED) {
				// Write it both in the log and to std::cout
				EZ_LOG(lg_, loglib::trace) << "ending processing before date " << curr_date << " and page " << i;
				std::cout << "ending processing before date " << curr_date << " and page" << i;	
				goto ESCAPE_NESTED_FOR_LOOPS;
			}

			// Build up URL to then fetch with cURL
			strBuilder.str("");
			strBuilder.clear();
			strBuilder << "http://archives.chicagotribune.com/";
			strBuilder << curr_date.year() << "/";
			strBuilder << std::setfill('0') << std::setw(2);
			strBuilder << curr_date.month().as_number() << "/";
			strBuilder << std::setfill('0') << std::setw(2);
			strBuilder << curr_date.day() << "/";
			strBuilder << "page/";
			strBuilder << i << "/xxlarge.jpg";

			std::string url;
			strBuilder >> url;
			
			std::unique_ptr<std::string> buffer(new std::string());
			urlFetcher.fetchUrl(buffer.get(), url);

			// Check to make sure download succeeded
			if (buffer->size()==0) {
				std::cerr << "File download failed! Attempted URL:" << std::endl;
				std::cerr << url << std::endl;
				continue;
			}
			
			// Important!  Will not work otherwise
			strBuilder.clear();

			// Construct name for bufferStruct.  Example: "chicagoT-2013-09-23-p3"
			strBuilder << "lstm-chicagoT-";
			strBuilder << curr_date.year() << "-";
			strBuilder << std::setfill('0') << std::setw(2);
			strBuilder << curr_date.month().as_number() << "-";
			strBuilder << std::setfill('0') << std::setw(2);
			strBuilder << curr_date.day() << "-";
			strBuilder << "p" << i;
			std::string bufferStructName;
			strBuilder >> bufferStructName;

			bufferStruct *buffer_struct = new bufferStruct(std::move(buffer), bufferStructName);
			// Place a new unique_ptr to bufferStruct made from above buffer and bufferStructName in buffer_queue.			
			buffer_queue.enqueue( 
					std::unique_ptr<bufferStruct>(buffer_struct) );
			cout << "Added " << bufferStructName << " to queue" << endl;
		} 
	}
	
	// Escaping the nested for loops in case of a SIGINT, lets us safely and peacefully finish off the program
	// We also naturally get here once there are no more dates left to throw into the queue.
	ESCAPE_NESTED_FOR_LOOPS:

	// Let the buffer_munchers know there is no more buffer to munch, send into the queue killer instances.
	for(int i = 0; i < num_ocr_instances; ++i)
		buffer_queue.enqueue(std::unique_ptr<bufferStruct>(new bufferStruct(nullptr, std::string(""), true)));
	
	for(auto& thread : ocr_threads) {
		thread.join();
	}
	EZ_LOG(lg_, loglib::info) << "Finished all OCR, from " << start_date_str << " to " << end_date_str << " with " << num_pages_to_classify << " pages each ";
	return 0;
}
