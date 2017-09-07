#include <iostream>
#include <cstdlib>
#include <thread>
#include <vector>
#include <unordered_map>
#include <mmemory>
#include "logging.hpp"
#include "util.hpp"

#include <leptonica/allheaders.h>  // Contains all of the headers for leptonica functionality (class pix defined there)
#include <tesseract/renderer.h> // For renderer functionality
#include <string>
using std::string;
#include <tesseract/baseapi.h>// Contains the class definitions for tesseract::TessBaseApi.  Requires using std::string in unichar.h
// Include this file in order to use GlobalDawgCache()
///#include <tesseract/dict.h>
#include <tesseract/publictypes.h> // Include this in order to get access to tesseract settings enums


using std::cout;
using std::cerr;
using std::endl;
using std::string;
using loglib::debug;

std::string getString (const std::string &string, const dict &dictionary, const std::string &default_string = "") {
	auto found_string_it = dictionary.find(string);
	if (found_string_it != dictionary.end()) // Means that the setting exists
		return *found_string_it;
	else
		return default_string;
}

class NewspaperPageClassifier {
	public:
		NewspaperPageClassifier(dict* settings) : settings_(settings) {};
		void changeSettings(dict* settings);
		std::string findSetting(const std::string &string, const std::string &default_string="") {
			return getString(string, *settings_, default_string)
		int classifyFile(std::string file);
		int init();

		// No copying/assignment of this object.
		NewspaperPageClassifier(const NewspaperPageClassifier) = delete;
		NewspaperPageClassifier& operator=(const NewspaperPageClassifier&) = delete;

	private:
		std::unique_ptr<dict> settings_;
		bool is_initialized_;
		loglib::logger lg_;
		tesseract::TessBaseAPI api_;
		std::string currentFile_;
		std::string tessdata_parent_dir_;
		std::string language_;
		long int total_pages_rendered_
}

void NewspaperPageClassifier::ChangeSettings(dict* settings) {
	for(auto const& setting : settings) 
		settings_->insert(setting);
	return;
}	

// Performs OCR on a file given as a buffer in an std::string ??
int NewspaperPageClassifier::classifyFile(std::string& buffer, std::string name = "") {
	
	// Initialize a renderer with a proper file name.
	if (name.empty()) {
		name = findSetting("Session Name")
		name +=	" : ";
		name += file;
		name += " : ";
		name += std::to_string(total_pages_rendered_);
	}

	// TODO(andy): Check if hocr_font_info boolean can be set beforehand without needing extraction
	bool font_info;
	api_.GetBoolVariable("hocr_font_info", &font_info);
	std::unique_ptr<tesseract::TessResultRenderer> renderer = new tesseract::TessTsvRenderer(name);
	
	// Do the processing
	if(!renderer->BeginDocument(name)) {
		EZ_LOG(lg_, debug) << "Renderer initialization failure";
		return -1;
	}
	// TODO(andy): Move renderer initialization and closing to constructor and destructor, maybe? 
	const unsigned char* data = reinterpret_cast<const unsigned char*>(buffer.c_str());
	Pix *pix = pixReadMem(data, buffer.size());
	if(!ProcessPage(pix, total_pages_rendered_, currentFile_, NULL, 0, renderer.get())) {
		EZ_LOG(lg_, debug) << "Page processing failed";
		return -1;
	}	
	pixDestroy(&pix);
	if(!renderer->EndDocument()) {
		EZ_LOG(lg_, debug) << "Renderer destruction failure";
		return -1;
	return 0;	
}


// Performs OCR on a file given as a filename
int NewspaperPageClassifier::classifyFile(std::string file, std::string name = "") {
	
	// Initialize a renderer with a proper file name.
	if (name.empty()) {
		name = findSetting("Session Name")
		name +=	" : ";
		name += file;
		name += " : ";
		name += std::to_string(total_pages_rendered_);
	}

	// TODO(andy): Check if hocr_font_info boolean can be set beforehand without needing extraction
	bool font_info;
	api_.GetBoolVariable("hocr_font_info", &font_info);
	std::unique_ptr<tesseract::TessResultRenderer> renderer = new tesseract::TessTsvRenderer(name);
	
	// Do the processing
	if(!renderer->BeginDocument(name)) {
		EZ_LOG(lg_, debug) << "Renderer initialization failure";
		return -1;
	}
	// TODO(andy): Move renderer initialization and closing to constructor and destructor, maybe? 
	Pix *pix = pixRead(file);
	if(!ProcessPage(pix, total_pages_rendered_, currentFile_, NULL, 0, renderer.get())) {
		EZ_LOG(lg_, debug) << "Page processing failed"; 
	pixDestroy(&pix);
	if(!renderer->EndDocument()) {
		EZ_LOG(lg_, debug) << "Renderer destruction failure";
		return -1;
	return 0;	
}

// Initializes the interal OCR classifier.  Call before ClassifyFile
int NewspaperPageClassifier::init() {
	lg_ = loglib::logger();
	loglib::init();
	EZ_LOG(lg_, debug) << "Tesseract start.  Session name: " << findSetting("Session Name");
	total_pages_rendered_ = 0;

	language_ = findSetting("Language", "eng");

	// Default tessdata directory should be /usr/local/share/tessdata/
	tessdata_parent_dir_ = findSetting("Tessdata Directory",
		*dict, "/usr/local/share/tessdata/");

	// OEM_TESSERACT_LSTM_COMBINED uses LSTM, but falls back on old TESSERACT if it struggles
	int failure = api_.Init(tessdata_parent_dir, language_,
			tesseract::OcrEngineMode::OEM_TESSERACT_LSTM_COMBINED);

	if (failure) {
		EZ_LOG(lg_, debug) << "Tessearct Failed to Init().  Aborting initialization";
		return -1;}
	
	// For parsing newspaper pages, PSM_AUTO is most appropriate page segmentation mode.
	api_.SetPageSegMode(tesseract::PageSegMode::PSM_AUTO);
	
	EZ_LOG(lg_, debug) << "Tesseract initialization successful";
	is_initialized_ = true;
	return 0;
}

// Function that runs_ocr on elements from a SharedQueue of buffers. 
void buffer_muncher(SharedQueue<std::unique_ptr<std::string> >& buffer_queue, std::string name, int pid) {
	dict settings = {{"Session Name", name.c_str()}};
	NewspaperPageClassifier ocr = NewspaperPageClassifier(&settings);
	while (true) {
		std::unique_ptr<std::string> buffer = std::move(buffer_queue->front());
		buffer_queue->pop_front();
	}

 

}

int main(int argn, const char** argv) {	
	using namespace boost::gregorian;
	const string start_date_str("1935-09-09");		// Start Date
	const string end_date_str("1935-09-20");		// End date

	const date start_date(from_simple_string(start_date_str));
	const date end_date(from_simple_string(end_date_str));

	const int num_tess_instances = 4;				// Number of tesseract instances
	for(int i = 0; i < num_tess_instances; ++i) {	
		classifier.push_back(std::unique_ptr<NewspaperPageClassifier>(new NewspaperPageClassifier()));
		classifier.back().get().init();				// Initialize object just push_back'd into std::vector.
	}

	ApiFetch urlFetcher();
	urlFetcher.init();
	
	SharedQueue<std::string> buffer_queue;
	const int buffer_queue_max_size = 2;		// Doesn't need to be very large.

	const int num_pages_to_classify = 4;		// How many pages deep to go into every daily issue
	stringstream URLBuilder;
	for(date curr_date = start_date; curr_date != end_date; curr_date += days(1)) {
		for(int i = 1; i <= num_pages_to_classify; i++) {
			URLbuilder << "http://archives.chicagotribune.com/";
			URLbuilder << curr_date.year() << "/";
			URLbuilder << std::setfill('0') << std::setw(2);
			URLbuilder << curr_date.month().as_number() << "/";
			URLbuilder << std::setfill('0') << set::setw(2);
			URLbuilder << curr_date.day() << "/";
			URLbuilder << "page/";
			URLbuilder << i << "/large.jpg";

			std::string url;
			URLbuilder >> url;
		
			std::unique_ptr<std::string> buffer(new std::string());
			fetchUrl(buffer.get(), std::string URL);

			// If buffer_queue doesn't need more stuff right now, thread will pause here on this func. call			
			buffer_queue.push_back_conditional(std::move(buffer), buffer_queue_max_size);
		}
	}

	// Here, we then insert four null pointers into the queue so that all tess instances know to shut down
	for(int i = 0; i < num_tess_instances; ++i)
		buffer_queue.push_back_conditional(std::unique_ptr<std::string>(nullptr));
	
	return 0;
}
