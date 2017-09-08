#ifndef OCR_WRAPPER_HPP_
#define OCR_WRAPPER_HPP_

#include "util.hpp"
#include <string>
#include <memory>
#include <tesseract/baseapi.h>
#include "logging.hpp"


// Wrapper for tesseract object performing OCR
// This class can take both a filename to an image, and a const char* buffer contained within
// a string as inputs, through the classifyFile methods.  
class OcrWrapper {
	public:
		// Initialize an OcrWrapper object with given settings.  All settings are optional.
		// 
		// Currently supported settings are:
		//	Session Name
		//		A string used for naming files saved by classifyFile if it isn't itself
		//		passed a string to name the output.
		//	Language
		//		Defaults to english (eng).  Can be set to change OCR to expect other languages.
		//		See tesseractocr github for language names and options
		//	Tessdata Directory
		//		Directory where to find tesseract's tessdata files.  Use this setting if
		//		your tessdata directory is not the default /usr/local/share/tessdata/
		//	
		//	Example of settings usage:
		//	dict settings = {{"Language", "eng"},{"Session Name","thisSetting"}}
		//
		OcrWrapper(dict* settings);
		
		// Iterates through settings given in, and individually modifies interal settings_
		// to match. Does not replace nor delete internal settings_, only updates (or creates)
		// key-value pairs in settings_.
		void changeSettings(dict* settings);

		// Helper method that returns a value if the key std::string is in settings_, elsewise
		// it returns the default_string given
		inline std::string findSetting(const std::string &string, const std::string &default_string="");
		inline std::string findSetting(const std::string &string, const char* string_literal);
		// Performs OCR on a file passed in by name, saving it in name
		int classifyFile(std::string file, std::string name="");
		
		// Performs OCR on a file passed in as an std::string buffer, saving it
		// in file name.  Buffer object should persist until function returns.
		int classifyFile(std::string* buffer, std::string name="");
		int init();

		// No copying/assignment of this object.
		OcrWrapper(const OcrWrapper&) = delete;
		OcrWrapper& operator=(const OcrWrapper&) = delete;

	private:
		bool has_been_initialized_;
		std::unique_ptr<dict> settings_;
		loglib::logger lg_;
		tesseract::TessBaseAPI api_;
		std::string currentFile_;
		std::string tessdata_parent_dir_;
		std::string language_;
		long int total_pages_rendered_;
};
#endif // OCR_WRAPPER_HPP_
