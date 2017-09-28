#include "util.hpp"
#include <iostream>
#include <getopt.h>

namespace util {

std::string getStringFromDict (const std::string &string, const util::dict &dictionary, const std::string &default_string) {
	auto found_string_it = dictionary.find(string);
	if (found_string_it != dictionary.end()) // Means that the setting exists
		return found_string_it->second;
	else
		return default_string;
}

void print_help() {
	std::cout <<
		"disjoint-pineapple\n"
		"\n"
		"Synopsis:\n"
		"	disjoint-pineapple [OPTIONS] DATE1 DATE2\n"
		"\n"
		"Description:\n"
		"	Program for searching through issues of the Chicago Tribune between the dates DATE1 and\n"
		"	DATE2, inclusive, with the dates given in YYYY-MM-DD format, and running OCR on them\n"
		"\n"
		"Usage:\n"
		"	disjoint-pineapple 1859-12-15 1865-03-30\n"
		"	disjoint-pineapple -o 4 1912-03-22 1915-10-09\n"
		"	disjoint-pineapple -b 10 -o 1 1860-01-01 1860-12-30\n"
		"	disjoint-pineapple --num_ocr_instances 3 --num_pages_to_classify 5 1933-04-01 1933-04-29\n"
		"\n"
		"Options:\n"
		"	-o --num_ocr_instances			Number of OCR instances to run.  Should not exceed number\n"
		"		of cores on machine.  Set by default to 2.\n"
		"	-b --buffer_queue_max_size		This value will almost never have to be modified.  However,\n"
		"		in the case of a very unstable/unreliable connection, this value can be increased so\n"
		"		that more files are pre-emptively downloaded.									\n"
		"	-n --num_pages_to_classify		Sets the number of pages to classify per newspaper issue.\n"
		"		Typically the most important content is in the first few pages, so this is set to a\n"
		"		default of 2.  Can be increased if you want to parse more newspaper pages per input\n"
		"	-h --help\n"
		"		Prints this help message\n";
}



bool parse_args(int argc, char* const* argv, dict& parsed_args) {
	
	struct option long_options[] =
	{
		{"help",					no_argument,		0, 'h'},	
		{"num_ocr_instances",		optional_argument,	0, 'o'},
		{"buffer_queue_max_size",	optional_argument,	0, 'b'},
		{"num_pages_to_classify",	optional_argument,	0, 'n'},
		{0, 0, 0, 0}
	};

	char c = 0;
	int option_index = 0;
	while( (c = getopt_long(argc, argv, "ho:b:n:", long_options, &option_index)) !=-1 ){
		switch (c) {
			case 0:
			break;
			case 'o':
				if(atoi(optarg)==0) {
					std::cout << "num_ocr_instances must be integer > 0 .  See --help for usage advice" << std::endl;
				} else {
					parsed_args["num_ocr_instances"] = optarg;
				} 
				break;
			case 'b':
				if(atoi(optarg)==0) {
					std::cout << "buffer_queue_max_size must be integer > 0.  See --help for usage advice" << std::endl;
				} else {
					parsed_args["buffer_queue_max_size"] = optarg;
				}
				break;
			case 'n':
				if(atoi(optarg)==0) {
					std::cout << "num_pages_to_classify must be integer > 0.  See --help for usage advice" << std::endl;
				} else {
					parsed_args["num_pages_to_classify"] = optarg;
				}
				break;
			case 'h':
				print_help();
				return false;
			default:
				std::cout << "Unsuccessful argument parse.  Type --help as argument to see usage";
				return false;
		}
	}

	if(argc - optind != 2) {
		std::cout << "wrong number of position argumens (the dates) given.  See --help for syntax expectations" << std::endl;
		return false;
	} else {
		parsed_args["start_date"] = argv[optind];
		parsed_args["end_date"] = argv[optind+1];
	}
	return true;
}
} // namespace util
