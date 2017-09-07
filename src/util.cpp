#include"util.hpp"
#include<queue>
#include<mutex>
#include<condition_variable>

template <typename T>
SharedQueue<T>::SharedQueue(){}

template <typename T>
SharedQueue<T>::~SharedQueue(){}

/* Diverges from usual definition of ::front() in a deque by pausing
 * thread asking for element until an element appears */
template <typename T>
T& SharedQueue<T>::front()
{
	std::unique_lock<std::mutex> mlock(mutex_);
	while (queue_.empty()) {
		cond_.wait(mlock);
	}

	return queue_.front();
}

/* see ::front() ; In case of empty queue waits for queue to gain
 * an element */
template <typename T>
void SharedQueue<T>::pop_front()
{
	std::unique_lock<std::mutex> mlock(mutex_);
	while (queue_.empty()) {
		cond_.wait(mlock);
	}
	queue_.pop_front();
	mlock.unlock();
	cond_2_.notify_one();	// notify the queue inputters that an element has been consumed
}     

/* Notify any waiting threads that an element has been added */
template <typename T>
void SharedQueue<T>::push_back(const T& item)
{
	std::unique_lock<std::mutex> mlock(mutex_);
	queue_.push_back(item);
	mlock.unlock();     // unlock before notificiation to minimize mutex con
	cond_.notify_one(); // notify one waiting thread

}

/* Notify any waiting threads that an element has been added */
template <typename T>
void SharedQueue<T>::push_back(T&& item)
{
	std::unique_lock<std::mutex> mlock(mutex_);
	queue_.push_back(std::move(item));
	mlock.unlock();     // unlock before notificiation to minimize mutex con
	cond_.notify_one(); // notify one waiting thread

}

/* Pushes onto the queue only if quantity < size(queue); locks calling thread until needed */
void SharedQueue<T>::push_back_conditional(const T& item, int goal)
{
	std::unique_lock<std::mutex> mlock(mutex_);
	while (queue_.size() >= goal) { 
		cond_2_.wait(mlock);
	}
	queue_.push_back(item);
	mlock.unlock();
	cond_.notify_one();
}

/* Pushes onto the queue only if quantity < size(queue); locks calling thread until item needed */
void SharedQueue<T>::push_back_conditional(T&& item, int goal)
{
	std::unique_lock<std::mutex> mlock(mutex_);
	while (queue_.size() >= goal) {
		cond_2_.wait(mlock);
	}
	queue_.push_back(std::move(item));
	mlock.unlock();
	cond_.notify_one();
}
	
template <typename T>
int SharedQueue<T>::size()
{
	std::unique_lock<std::mutex> mlock(mutex_);
	int size = queue_.size();
	mlock.unlock();
	return size;
}
