#include <iostream>
#include <cstdlib>

// includes:
// allheaders.h
// baseapi.h
// basedir.h
// dict.h
// openclwrapper.h
// renderer.h
// simdetect.h
// strings.h
// tprintf.h

// Contains all of the headers for leptonica functionality (class pix defined there)
#include "allheaders.h"

// For renderer functionality
#include "renderer.h"

// Contains the class definitions for tesseract::TessBaseApi.  Requires using std::string in unichar.h
#include<string>
using std::string;
#include "baseapi.h"

// Requires some complicated nonsense involving dict.h which you can't get from include headers
// Include this file in order to use GlobalDawgCache()
///#include "dict.h"

// Include this in order to get access to tesseract settings enums
#include "publictypes.h"


using std::cout;
using std::cerr;
using std::endl;

int main() {
	// Call to create the DawgCache object before TessBaseAPI object
	// to ensure proper order of destructor calls when exiting function
	// tesseract::Dict::GlobalDawgCache();

	// This is how tesseract's command line utility initializes tesseract.  By making
	// it static, we place it in the data segment instead of the stack, which is
	// useful sometimes for large variables.
	// static tesseract::TessBaseAPI api;

	tesseract::TessBaseAPI api;

	const char* outputName = "purr_TESS_ONLY";
	api.SetOutputName(outputName);

	// Location of tessdata directory
	const char* tessdata_parent_dir = "/usr/local/share/tessdata/";

	// Language to be used.  We want English.
	const char* language = "eng";

	// Filename of image to be processed.
	const char* image = "./seq-2.jp2";

	// Engine mode.  We want to use both the neural network and tesseract classic
	// to fall back on if it doesn't work out.
	//auto engineMode = tesseract::OcrEngineMode::OEM_TESSERACT_LSTM_COMBINED;
	auto engineMode = tesseract::OcrEngineMode::OEM_TESSERACT_ONLY;

	int ret = api.Init(tessdata_parent_dir, NULL, engineMode);

	if (ret) {
		cout << "Tesseract intialization failure, exiting" << endl;
		return EXIT_FAILURE;
	}

	// Page segmentation mode.  There's quite a few options here, but it seems
	// that PM_AUTO_OSD does everything, which is ideal.
	// If we do the splitting of text into columns on our own, we'd call
	// PSM_SINGLE_COLUMN instead
	api.SetPageSegMode(tesseract::PageSegMode::PSM_AUTO_OSD);

	/*
	 * Only needed if image isn't being set elsewhere
	 *
	Pix* pix_image = pixRead(image);
	if(!pix_image) {
		cerr << "Could not read image file: " << image << endl;
		return EXIT_FAILURE;
	}
	
	api.setImage(pix_image);
	*/

	bool font_info;
	api.GetBoolVariable("hocr_font_info", &font_info);
	tesseract::TessResultRenderer* renderer = new tesseract::TessTsvRenderer(outputName);

	bool success = api.ProcessPages(image, NULL, 0, renderer);
	if(!success) {
		cout << "Processing stage failed." << endl;
		return EXIT_FAILURE;
	}
}
