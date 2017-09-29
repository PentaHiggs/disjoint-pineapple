# disjoint-pineapple

Simple project to familiarize myself with Tesseract, cURL, and the like.  Currently, the program is capable of scraping
newspaper articles from the website for the Chicago Tribune *(see below), performing OCR on them, and saving the files
in an appropriate format.  The difference between this home-made OCR and the kind that is usually made available for you
is that I save it in a format which also comes along with extra metadata, currently limited to word certainty scores, so that
if running machine learning algorithms on the data, low-certainty words can be excluded.

[*!!!NOTE!!! :  the Chicago Tribune was initially chosen because of its relevancy and because it allowed access to high-quality
images of its newspapers for free.  This is no longer the case; for a month's viewing of images you now are forced to
cough up ~$7 US for the privelege of viewing.  Thus, this program will now fail at doing the very job it was designed to do
unless you decide to suscribe to their newspaper viewing service and modify the cURL code to allow for logging in.  This
modification will, hopefully, be made soon]

## Usage

Using this program is as simple as

``` sh
./bin/main 1930-10-12 1940-05-26
```

With this command you will automatically start scraping the Chicago Tribune website for large images of its newspaper
pages and start performing OCR on them, between the given dates, inclusive.  A keyboard interrupt (CTRL-c) can be used
to stop adding images to the queue and let the Tesseract instances finish their current work and peacefully shut down
(note, this may take a few minutes).  To see additional options, feel free to call the program with --help

``` sh
./bin/main --help
```

## Getting Started

If for whatever reason you wish to get your own copy of this project running, do not despair!  These are the things you will need

### Prerequisites

There are unfortunately a large amount of dependencies, and as of the time of writing, some of them will require compilation.

#### Linux

The code was compiled under Ubuntu 16.04 64-bit, but any flavor of Linux that can manage to compile Tesseract (see below)
should have no trouble compiling this code.  The code is currently not Windows compatible, but most of the dependencies
are, except the handling of command-line arguments through the POSIX function getopt().  

#### Tesseract

This code is written to work with Tesseract 4.0 or greater.  It is currently in beta status and is being actively developed,
so as of now you have no choice but to have to compile it.  All of the versions that come in system package managers are
of the 3.x variety, which do not have the newest LSTM-based word recognition systems.  Accuracy is greatly improved,
especially in the face of uncertain classifications, using this system, so I highly recommend employing it.  See

https://github.com/tesseract-ocr/tesseract

for installation details.  Also, for best performance, it is recommended that multi-threading for Tesseract be turned off
when configuring the Tesseract source for compilation.  Until TesseractOCR is coded to make use of GPUs, it is more efficient
to have several single-threaded instances of Tesseract running at the same time than to have one multithreaded instance
in the case of classifiying a large corpus of images/pages.

#### cURL

cURL is the software library used for http requests, instructions on how to install on your platform can be obtained at

https://curl.haxx.se/

Oftentimes, your package manager of choice will have a perfectly adequate verson of cURL for you to use.  On Ubuntu 16.04,
you can install cURL via

``` sh
sudo apt-get install curl
```

#### Boost

This project makes use of the Boost logging and testing libraries.  More information about Boost can be found below.  One
can choose to either install the entire set of Boost libraries, or just the test and logging libraries and their dependencies.

http://www.boost.org/doc/libs/1_65_1/more/getting_started/unix-variants.html

Note: Unix-based systems' package managers ofentimes have their own version of Boost, and it should be perfectly adequate
for this project.  For example, to install all of the boost libraries on Ubuntu,

``` sh
sudo apt-get install libboost-all-dev
```

### Installing

This project was built using g++ version 5.4, but any c++ compiler with support for c++14 should be fine (And honestly,
just support for c++11 should be fine too).  Getting the code working, after installing prerequisites, is as simple as

``` sh
git clone https://github.com/PentaHiggs/disjoint-pineapple.git
cd disjoint-pineapple
make main
```
## Running the tests

There are currently not many tests, but if you wish to make and run them, enter into the code's base directory and type

``` sh
make test
make runtest
```

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details
