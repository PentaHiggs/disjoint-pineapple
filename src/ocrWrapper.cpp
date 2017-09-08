#include <string>
#include <memory>
#include <tesseract/baseapi.h>
#include <tesseract/renderer.h>
#include <leptonica/allheaders.h>
#include "ocrWrapper.hpp"
#include "logging.hpp"
#include "util.hpp"

using loglib::debug;

// Helper function for searching in a dictionary for a value with key std::string string.
// Returns std::string default_string if such a value does not exist.
std::string getStringFromDict (const std::string &string, const dict &dictionary, const std::string &default_string = "") {
	auto found_string_it = dictionary.find(string);
	if (found_string_it != dictionary.end()) // Means that the setting exists
		return found_string_it->second;
	else
		return default_string;
}

OcrWrapper::OcrWrapper(dict* settings) : has_been_initialized_(false),
	settings_(settings) {}

// Searches internal settings dictionary for a specified setting, returning it
// If Setting cannot be found, default_string is returned in its place.
inline std::string OcrWrapper::findSetting(const std::string &string, 
		const std::string &default_string) {
	return getStringFromDict(string, *settings_, default_string);
}
inline std::string OcrWrapper::findSetting(const std::string &string,
		const char* string_literal) {
	return getStringFromDict(string, *settings_, std::string(string_literal));
}


int OcrWrapper::classifyFile(std::string *buffer, std::string name) {
	// In case user forgot to init.  init() is not thread-safe
	init();
	// Initialize a renderer with a proper file name.
	if (name.empty()) {
		name = findSetting("Session Name");
		name +=	" : ";
		name += "[FROM BUFFER]";
		name += " : ";
		name += std::to_string(total_pages_rendered_);
	}

	// Passing in name.c_str() is OK; renderer is killed before string goes out of scope
	std::unique_ptr<tesseract::TessResultRenderer> renderer(
		new tesseract::TessTsvRenderer(name.c_str()));
	
	// Do the processing
	if(!renderer->BeginDocument(name.c_str())) {
		EZ_LOG(lg_, debug) << "Renderer initialization failure";
		return -1;
	}
	// TODO(andy): Move renderer initialization and closing to constructor and destructor, maybe? 
	const unsigned char* data = reinterpret_cast<const unsigned char*>(buffer->c_str());
	Pix *pix = pixReadMem(data, buffer->size());
	if(!api_.ProcessPage(pix, 0, "", NULL, 0, renderer.get())) {
		EZ_LOG(lg_, debug) << "Page processing failed";
		return -1;
	}	
	pixDestroy(&pix);
	if(!renderer->EndDocument()) {
		EZ_LOG(lg_, debug) << "Renderer destruction failure";
		return -1;
	}
	++total_pages_rendered_; // Even if function call was not successful.
	return 0;
}

int OcrWrapper::classifyFile (std::string file, std::string name) {
	// In case user forgot to init.  init() is not thread-safe
	init();
	// Initialize a renderer with a proper file name.
	if (name.empty()) {
		name = findSetting("Session Name");
		name +=	" : ";
		name += file;
		name += " : ";
		name += std::to_string(total_pages_rendered_);
	}

	std::unique_ptr<tesseract::TessResultRenderer> renderer(
			new tesseract::TessTsvRenderer(name.c_str()));
	
	// Do the processing
	if(!renderer->BeginDocument(name.c_str())) {
		EZ_LOG(lg_, debug) << "Renderer initialization failure";
		return -1;
	}
	// TODO(andy): Move renderer initialization and closing to constructor and destructor, maybe? 
	Pix *pix = pixRead(file.c_str());
	if(!api_.ProcessPage(pix, 0, "", NULL, 0, renderer.get())) {
		EZ_LOG(lg_, debug) << "Page processing failed"; 
	}
	pixDestroy(&pix);
	if(!renderer->EndDocument()) {
		EZ_LOG(lg_, debug) << "Renderer destruction failure";
		return -1;
	}
	++total_pages_rendered_;  // Even if function call was not successful.
	return 0;	
}

// Initializes the interal OCR classifier.  Call before classifyFile, but after initialization.
// Is safe to be called more than once.
// Not necessarily thread-safe, multiple init()s running concurrently may cause trouble.
int OcrWrapper::init() {
	// In case init() has already been called.
	if (has_been_initialized_)
		return 0;
	loglib::init();
	lg_ = loglib::logger();
	
	EZ_LOG(lg_, debug) << "Tesseract start.  Session name: " << findSetting("Session Name");
	total_pages_rendered_ = 0;

	language_ = findSetting("Language", "eng");

	// Default tessdata directory should be /usr/local/share/tessdata/
	tessdata_parent_dir_ = findSetting("Tessdata Directory", "/usr/local/share/tessdata/");

	// OEM_TESSERACT_LSTM_COMBINED uses LSTM, but falls back on old TESSERACT if it struggles
	int failure = api_.Init(tessdata_parent_dir_.c_str(), language_.c_str(),
			tesseract::OcrEngineMode::OEM_TESSERACT_LSTM_COMBINED);

	if (failure) {
		EZ_LOG(lg_, debug) << "Tessearct Failed to Init().  Aborting initialization";
		return -1;}
	
	// For parsing newspaper pages, PSM_AUTO is most appropriate page segmentation mode.
	api_.SetPageSegMode(tesseract::PageSegMode::PSM_AUTO);
	
	EZ_LOG(lg_, debug) << "Tesseract initialization successful";
	has_been_initialized_ = true;
	return 0;
}
