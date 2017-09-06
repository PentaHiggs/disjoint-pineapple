#ifndef API_FETCH_H
#define API_FETCH_H

#include<string>
#include<curl/curl.h>
#include<memory>
#include"logging.hpp"

class ApiFetch {
	public:
		ApiFetch();
		~ApiFetch();
		bool init();
		bool fetchUrl(std::string* buffer, std::string URL);
	private:
		CURL* conn_;
		loglib::logger lg_;
		string errorBuffer;
		static size_t writer(char*, size_t, size_t, std::string);
};
#endif //API_FETCH_H
