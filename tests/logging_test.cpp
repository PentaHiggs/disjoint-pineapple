#include"logging.hpp"

int main(int, char*[]) {
	loglib::init();
	loglib::logger lg;
	
	EZ_LOG(lg, loglib::debug) << "Keep";
	EZ_LOG(lg, loglib::info) << "It";
	EZ_LOG(lg, loglib::warning) << "Simple";
	EZ_LOG(lg, loglib::error) << "Stupid";
	
	return 0;
}

