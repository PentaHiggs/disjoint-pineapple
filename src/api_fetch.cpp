#include<string>
#include<iostream>
#include<curl/curl.h>

// Useful using directives.  Not importing entire std namespace to avoid trouble
using std::cerr;
using std::cout;
using std::endl;
using std::string;

using namespace std;

// TODO: cURL doesn't actually expect globals.  I was just stupid.
// curl expects to see these global variables.  Declared static so they stay here
static char errorBuffer[CURL_ERROR_SIZE];
static string buffer;


// libcurl write callback function
static int writer(char* data, size_t size, size_t nmemb, string* writerData)
{
	//size_t written = fwrite(ptr,size,mnemb, (FILE*) stream);
	if(writerData == NULL)
		return 0;
	writerData->append(data, size * nmemb);
	return size * nmemb;
}


static bool init(CURL* &conn, string &url) {
	
	CURLcode code;
	conn = curl_easy_init();

	if(conn==NULL) {
		cerr << "cURL connection initialization failed" << endl;
		return false;
	}
	
	code = curl_easy_setopt(conn, CURLOPT_ERRORBUFFER, errorBuffer);
	if(code != CURLE_OK) {
		cerr << "cURL failed to set error buffer " << code << endl;
		return false;
	}

	//curl_easy_setopt(conn, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(conn, CURLOPT_NOPROGRESS, 1L);

	code = curl_easy_setopt(conn, CURLOPT_URL, url.c_str());
	if(code != CURLE_OK) {
		cerr << "cURL failed to properly set URL " << url << endl;
		return false;
	}
	
	
	code = curl_easy_setopt(conn, CURLOPT_FOLLOWLOCATION, 1L);
	if(code != CURLE_OK) {
		cerr << "cURL failed to set redirect option " << errorBuffer << endl;
		return false;
	}

	code = curl_easy_setopt(conn, CURLOPT_WRITEFUNCTION, writer);
	if(code != CURLE_OK) {
		cerr << "cURL failed to set writer " << errorBuffer << endl;
		return false;
	}

	code = curl_easy_setopt(conn, CURLOPT_WRITEDATA, &buffer);
	if(code != CURLE_OK) {
		cerr << "cURL failed to write data " << errorBuffer << endl;
		return false;
	}

	// Everything succeeded
	cout << "cURL was successfully initialized on url " << url << endl;
	return true;
}

int main(int argc, char** argv) {
	CURL *conn = NULL;
	CURLcode code;
	string out;
	
	if(argc != 2) {
		cerr << "Usage: " << argv[0] << " <url>" << endl;
		return EXIT_FAILURE;
	}
	string url = argv[1];
	curl_global_init(CURL_GLOBAL_DEFAULT);

	// Connection failed.  init function returns mode of failure
	if(!init(conn, url)) {
		return EXIT_FAILURE;
	}

	code = curl_easy_perform(conn);
	if(code != CURLE_OK) {
		cerr << "Failed to get '" << argv[1] << "' [" << errorBuffer << "]" << endl;
		cerr << "Contents of buffer are " << buffer << endl;
		curl_easy_cleanup(conn);
		return EXIT_FAILURE;
	}

	cout << "Successfully downloaded cURL url!.  Printing to terminal. " << endl;
	cout << buffer << endl;
	curl_easy_cleanup(conn);
	return EXIT_SUCCESS;
}
