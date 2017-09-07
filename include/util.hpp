#include<unordered_map>
#include<string>
#include<queue>
#include<mutex>
#include<condition_variable>

typedef std::unordered_map<std::string, std::string> dict;

// Implementation obtained from https://stackoverflow.com/questions/36762248/
//	why-is-stdqueue-not-thread-safe

template<typename T>
class SharedQueue
{
	public:
		SharedQueue();
		~SharedQueue();

		T& front();
		void pop_front();
		void push_back(const T& item);
        void push_back(T&& item);

        int size();
        bool empty();

    private:
        std::deque<T> queue_;
        std::mutex mutex_;
        std::condition_variable cond_;
		std::condition_variable cond_2_;
}; 
