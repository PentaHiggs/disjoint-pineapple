#define BOOST_TEST_MODULE util_test
#define BOOST_TEST_DYN_LINK

#include "util.hpp"
#include <boost/test/unit_test.hpp>
#include <vector>

std::string showDictDifference (util::dict reference, util::dict actual) {
	std::string differenceString;
	for(const auto& entry : reference) {
		auto in_other = actual.find(entry.first);
		if (in_other==actual.end()) {
			// Entry does not exist
			differenceString += entry.first;
			differenceString += " does not exist in dict\n";
		} else {
			// Key found
			if (entry.second != in_other->second) {
				// Value is unequal
				differenceString += "Value mismatch in ";
				differenceString += entry.first;
				differenceString += " ;  got ";
				differenceString += in_other->second;
				differenceString += " while expecting ";
				differenceString += entry.second;
				differenceString += "\n";
			}
		// No problem encountered
		}
	}
	return differenceString;
}

BOOST_AUTO_TEST_CASE(parse_args_plain)
{
	char arguments_[][64] = {
		"disjoint-pineapple",
		"1930-12-10", "1931-04-09"};
	size_t nArgs = 3;

	// getopt is fucking stupid, holy shit.  Only reason I need to do this.  Terrible interface
	char* arguments[nArgs];
	for (size_t i = 0; i < nArgs; i++) {arguments[i] = arguments_[i];}
	
	util::dict expected_output = {
		{"start_date",			"1930-12-10"},
		{"end_date",			"1931-04-09"}
	};

	util::dict actual_output;
	bool success = util::parse_args(nArgs, arguments, actual_output);
	
	BOOST_CHECK_MESSAGE( success, "parse_args failed on basic input");
	BOOST_CHECK_MESSAGE( expected_output == actual_output, showDictDifference(expected_output, actual_output));
}

BOOST_AUTO_TEST_CASE(parse_args_with_options)
{
	char arguments_[][64] = {
		"disjoint-pineapple",
		"-o", "8",
		"-b", "2",
		"-n", "7",
		"1924-12-14", "1930-01-29"};

	size_t nArgs = 9;

	// Same workaround for getopt
	char* arguments[nArgs];
	for (size_t i=0; i < nArgs; i++) {arguments[i] = arguments_[i];}
	
	util::dict expected_output = {
		{"num_ocr_instances",		"8"},
		{"buffer_queue_max_size",	"2"},
		{"num_pages_to_classify",	"7"},
		{"start_date",				"1924-12-14"},
		{"end_date",				"1930-01-29"}
	};

	util::dict actual_output;
	bool success = util::parse_args(nArgs, arguments, actual_output);

	BOOST_CHECK_MESSAGE( success, "parse_args failed on input with options" );
	BOOST_CHECK_MESSAGE( expected_output == actual_output , showDictDifference(expected_output, actual_output));

}

