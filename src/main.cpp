#include <iostream>
#include <cstdlib>
#include "logging.hpp"

#include <leptonica/allheaders.h>  // Contains all of the headers for leptonica functionality (class pix defined there)
#include <tesseract/renderer.h> // For renderer functionality
#include<string>
using std::string;
#include <tesseract/baseapi.h>// Contains the class definitions for tesseract::TessBaseApi.  Requires using std::string in unichar.h
// Include this file in order to use GlobalDawgCache()
///#include <tesseract/dict.h>
#include <tesseract/publictypes.h> // Include this in order to get access to tesseract settings enums

#include<unordered_map>
#include<memory>

using std::cout;
using std::cerr;
using std::endl;
using std::string;

typedef std::unordered_map<std::string, std::string> dict;

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

void NewspaperPageClassifier::ChangeSettings {
	for(auto const& setting : settings) 
		settings_->insert(setting);
	return;
}	


// TODO(andy): Specialize this function to use URLS and load pix files from FILE objects?
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
		EZ_LOG(lg_, "Renderer initialization failure");
		return -1;
	}
	// TODO(andy): Move renderer initialization and closing to constructor and destructor, maybe? 
	Pix *pix = pixRead(file);
	if(!ProcessPage(pix, total_pages_rendered_, currentFile_, NULL, 0, renderer.get())) {
		EZ_LOG(lg_, 
	pixDestroy(&pix);
	if(!renderer->EndDocument()) {
		EZ_LOG(lg_, "Renderer destruction failure");
		return -1;
	return 0;	
}

// Initializes the interal OCR classifier.  Call before ClassifyFile
int NewspaperPageClassifier::init() {
	lg_ = loglib::logger();
	loglib::init();
	EZ_LOG(lg_, "Tesseract start.  Session name: "
				+ findSetting("Session Name"));
	total_pages_rendered_ = 0;

	language_ = findSetting("Language", "eng");

	// Default tessdata directory should be /usr/local/share/tessdata/
	tessdata_parent_dir_ = findSetting("Tessdata Directory",
		*dict, "/usr/local/share/tessdata/");

	// OEM_TESSERACT_LSTM_COMBINED uses LSTM, but falls back on old TESSERACT if it struggles
	int failure = api_.Init(tessdata_parent_dir, language_,
			tesseract::OcrEngineMode::OEM_TESSERACT_LSTM_COMBINED);

	if (failure) {
		EZ_LOG(lg_, "Tessearct Failed to Init().  Aborting initialization");
		return -1;}
	
	// For parsing newspaper pages, PSM_AUTO is most appropriate page segmentation mode.
	api_.SetPageSegMode(tesseract::PageSegMode::PSM_AUTO);
	
	EZ_LOG(lg_, "Tesseract initialization successful");
	is_initialized_ = true;
	return 0;
}


int main(int argn, const char** argv) {
	
	NewspaperPageClassifier npc;
	npc.init();
	
}
