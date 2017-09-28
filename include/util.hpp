#ifndef UTIL_HPP_
#define UTIL_HPP_

#include<unordered_map>
#include<string>
#include<queue>
#include<mutex>
#include<condition_variable>
#include<climits>
#include<vector>

namespace util {

typedef std::unordered_map<std::string, std::string> dict;

// Helper function for searching in a dictionary for a value with key std::string string.
// Returns std::string default_string if such a value does not exist.
std::string getStringFromDict (const std::string &string, const util::dict &dictionary, const std::string &default_string = ""); 


// Implements a thread-safe queue of limited length
// If the queue is empty, calls to dequeue wait on enqueue calls to repopulate the queue.
// If the queue is full, calls to enqueue wait on dequeue calls to empty the queue.
//
// Be warned, this queue's enqueue and dequeue methods block the calling function until
// they can successfully go through with insertion/deletion.  
template<typename T>
class SharedQueue
{
	public:
		SharedQueue(int length);
		~SharedQueue();

		// Pops the top element off of the queue, returning it with move semantics.
		// If the queue is empty, the querying thread is halted until a call to
		// enqueue populates the queue once more.
		T dequeue();

		// Pushes the given element onto the end of the queue.  If the queue is
		// full (has length_ items), then the current thread halts until a call
		// to dequeue reduces the current size and creates space.
		void enqueue(const T& item);

		// Pushes the given element onto the end of the queue.  If the queue is
		// full (has length_ items), then the current thread halts until a call
		// to dequeue reduces the current size and creates space.
        void enqueue(T&& item);

		// Returns the number of elements in queue (not the max length)
		int size();

		// Returns true of the queue is empty
        bool empty();

    private:
        std::deque<T> queue_;
        std::mutex mutex_;
        std::condition_variable available_consumer_;
		std::condition_variable available_item_;
		const int length_;
};

template <typename T>
SharedQueue<T>::SharedQueue(int length) 
	: length_((length <= 0) ? INT_MAX : length) {}

template <typename T>
SharedQueue<T>::~SharedQueue(){}

template <typename T>
T SharedQueue<T>::dequeue() {
	std::unique_lock<std::mutex> mlock(mutex_);
	while (queue_.empty()) {
		available_item_.wait(mlock);	// Give up mutex until enqueue happens
	}
	T retVal = std::move(queue_.front());	
	queue_.pop_front();
	mlock.unlock();						// Next announcement doesn't have to be synchronzied
	available_consumer_.notify_one();
	return retVal;
}

template <typename T>
void SharedQueue<T>::enqueue(const T& item) {
	std::unique_lock<std::mutex> mlock(mutex_);
	while (queue_.size() >= length_) {
		available_consumer_.wait(mlock);
	}
	queue_.push_back(item);
	mlock.unlock();
	available_item_.notify_one();
}

template <typename T>
void SharedQueue<T>::enqueue(T&& item) {
	std::unique_lock<std::mutex> mlock(mutex_);
	while (queue_.size() >= length_) {
		available_consumer_.wait(mlock);
	}
	queue_.push_back(std::move(item));
	mlock.unlock();
	available_item_.notify_one();
}
	
template <typename T>
int SharedQueue<T>::size(){
	std::unique_lock<std::mutex> mlock(mutex_);
	int size = queue_.size();
	mlock.unlock();
	return size;
}

template <typename T>
bool SharedQueue<T>::empty() {
	std::unique_lock<std::mutex> mlock(mutex_);
	bool empty = queue_.emoty();
	mlock.unlock();
	return empty;
}

/* --------------------- END SharedQueue<T> Implementation ----------------------- */

// Parses command-line arguments for the program and returns a dictionary with values
// as strings.  If strings need to be convertible to another type, such as int, the
// convertability is checked within this function but not performed.
bool parse_args(int argc, char* const* argv, dict& parsed_arguments);

// Prints a help statement indicating proper command-line usage of the program.
void print_help();

} // namespace util
#endif // UTIL_HPP_
