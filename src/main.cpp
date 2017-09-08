#include <iostream>
#include <thread>
#include <vector>
#include <unordered_map>
#include <memory>
#include <cassert>
#include <sstream>

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
#include<boost/date_time/gregorian/gregorian.hpp>

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using loglib::debug;

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
void buffer_muncher(SharedQueue< std::unique_ptr<bufferStruct> > *buffer_queue, int pid) {
	loglib::init();
	loglib::logger lg_();
	dict settings{};
	OcrWrapper ocr(&settings);
	ocr.init();
	// Keep munching buffers until you get a nullptr
	while (true) {
		// Following function will wait on buffer_queue to have bufferStruct elements to munch
		std::unique_ptr<bufferStruct> buffer_struct_ptr = std::move(buffer_queue->dequeue());
		if (!buffer_struct_ptr->kill_muncher) {
			assert(buffer_struct_ptr->buffer);	// Pointer should not be empty.
			ocr.classifyFile(buffer_struct_ptr->buffer.get(), buffer_struct_ptr->name);
			//EZ_LOG(lg_, loglib::debug) << "Munched " << buffer_struct_ptr->name;
		} else {	// we kill the muncher.
			//EZ_LOG(lg_, loglib::debug) << "Shutdown order received @ muncher " << pid;
			return;
		}
	// buffer_struct object naturally deallocated here, along with its buffer and name
	}
	return;
}

int main(int argn, const char** argv) {	
	loglib::init();
	loglib::logger lg_();

	using namespace boost::gregorian;
	const string start_date_str("1935-09-09");		// Start Date
	const string end_date_str("1935-09-20");		// End date

	const date start_date(from_simple_string(start_date_str));
	const date end_date(from_simple_string(end_date_str));

	const int buffer_queue_max_size = 2;		// Doesn't need to be very large.
	SharedQueue< std::unique_ptr<bufferStruct> > buffer_queue(buffer_queue_max_size);

	const int num_ocr_instances = 4;				// Number of OCR instances.  Increase if cores > 4
	std::vector<std::thread> ocr_threads;
	for(int i = 0; i < num_ocr_instances; ++i) {	
		ocr_threads[0] = std::thread(buffer_muncher, &buffer_queue, i);
	}

	ApiFetch urlFetcher = ApiFetch();
	urlFetcher.init();
	
	const int num_pages_to_classify = 4;		// How many pages deep to go into every daily issue
	std::stringstream strBuilder;
	for(date curr_date = start_date; curr_date != end_date; curr_date += days(1)) {
		for(int i = 1; i <= num_pages_to_classify; i++) {
			// Build up URL to then fetch with cURL
			strBuilder << "http://archives.chicagotribune.com/";
			strBuilder << curr_date.year() << "/";
			strBuilder << std::setfill('0') << std::setw(2);
			strBuilder << curr_date.month().as_number() << "/";
			strBuilder << std::setfill('0') << std::setw(2);
			strBuilder << curr_date.day() << "/";
			strBuilder << "page/";
			strBuilder << i << "/large.jpg";

			std::string url;
			strBuilder >> url;
		
			std::unique_ptr<std::string> buffer(new std::string());
			urlFetcher.fetchUrl(buffer.get(), url);

			// Construct name for bufferStruct.  Example: "chicagoT-2013-09-23-p3"
			strBuilder << "chicagoT-";
			strBuilder << curr_date.year() << "-";
			strBuilder << std::setfill('0') << std::setw(2);
			strBuilder << curr_date.month().as_number() << "-";
			strBuilder << std::setfill('0') << std::setw(2);
			strBuilder << curr_date.day() << "-";
			strBuilder << "p" << i;
			std::string bufferStructName;
			strBuilder >> bufferStructName;

			// Place a new unique_ptr to bufferStruct made from above buffer and bufferStructName in buffer_queue.			
			buffer_queue.enqueue( 
					std::unique_ptr<bufferStruct>(new bufferStruct(std::move(buffer), bufferStructName)) );
		} 
	}

	// Let the buffer_munchers know there is no more buffer to munch, send into the queue killer instances.
	for(int i = 0; i < num_ocr_instances; ++i)
		buffer_queue.enqueue(std::unique_ptr<bufferStruct>(new bufferStruct(nullptr, std::string(""), true)));
	return 0;
}
