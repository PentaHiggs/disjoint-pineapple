#include<string>
#include<iostream>
#include<curl/curl.h>
#include<memory>
#include"logging.hpp"
#include"api_fetch.hpp"

// Useful using directives.  Not importing entire std namespace to avoid trouble
using std::cerr;
using std::cout;
using std::endl;
using std::string;
using loglib::debug;

class ApiFetch {
	public:
		ApiFetch();
		~ApiFetch();
		bool init();
		bool fetchUrl(string* buffer, string URL);
	private:
		CURL* conn_;
		loglib::logger lg_;
		string errorBuffer;
		static size_t writer(char* data, size_t size, size_t nmemb, string* writerData);	
};

size_t ApiFetch::writer(char* data, size_t size, size_t nmemb, string* writerData)
{
	if(writerData == NULL)
		return 0;
	writerData->append(data, size * nmemb);
	return size * nmemb;
}

ApiFetch::ApiFetch() : conn_(nullptr) {}

ApiFetch::~ApiFetch() {
	if(!(conn_==nullptr) && !(conn_==NULL)) 
		curl_easy_cleanup(conn_);
}


bool ApiFetch::init() {	
	CURLcode code;
	conn_ = curl_easy_init();
	lg_ = loglib::logger();
	loglib::init();
	
	
	if(conn_==nullptr) {
		EZ_LOG(lg_, debug) << "cURL connection initialization failed";
		return false;
	}
	code = curl_easy_setopt(conn_, CURLOPT_ERRORBUFFER, errorBuffer);
	if(code != CURLE_OK) {
		EZ_LOG(lg_, debug) <<  "cURL failed to set error buffer ";
		return false;
	}

	curl_easy_setopt(conn_, CURLOPT_NOPROGRESS, 1L);
	code = curl_easy_setopt(conn_, CURLOPT_FOLLOWLOCATION, 1L);
	if(code != CURLE_OK) {
		EZ_LOG(lg_, debug) << "cURL failed to set redirect option " << errorBuffer;
		return false;
	}
	
	return true;

}


bool ApiFetch::fetchUrl(string* buffer, string URL) {
	CURLcode code;
	if ( (conn_==NULL) || (conn_==nullptr) ) {
	}
	code = curl_easy_setopt(conn_, CURLOPT_URL, URL.c_str());
	if(code != CURLE_OK) {
		EZ_LOG(lg_, debug) << "cURL failed to properly set URL " <<  URL;
		return false;
	}
	code = curl_easy_setopt(conn_, CURLOPT_WRITEDATA, buffer);
	if(code != CURLE_OK) {
		EZ_LOG(lg_, debug) << "cURL failed to write data " << errorBuffer;
		return false;
	}

	// TODO(andy): This code using std::bind refuses to work, causes segmentation faults, using static member function instead.
	// Why does this cause a segfault when running curl_easy_perform?
	/*using namespace std::placeholders;
	auto writer_func = std::bind(&ApiFetch::writer, this, _1, _2, _3, _4);
	code = curl_easy_setopt(conn_, CURLOPT_WRITEFUNCTION,
			writer_func);
	*/
	code = curl_easy_setopt(conn_, CURLOPT_WRITEFUNCTION, writer);
	if(code != CURLE_OK) {
		EZ_LOG(lg_, debug) << "cURL failed to set writer " << errorBuffer;
		return false;
	}
	code = curl_easy_perform(conn_);
	if(code != CURLE_OK) {
		EZ_LOG(lg_, debug) << "cURL file download from URL" << URL << "failed";
	}
	return true;
}


