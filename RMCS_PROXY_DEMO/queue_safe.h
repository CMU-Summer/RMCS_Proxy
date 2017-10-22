/*
 * threadsafe_queue.h
 *
 *  Created on: 2016年7月26日
 *      Author: guyadong
 */

#ifndef QUEUE_SAFE_H
#define QUEUE_SAFE_H
#include <queue>
#include <mutex>
#include <condition_variable>


/*
 * 线程安全队列
 * T为队列元素类型
 * 因为有std::mutex和std::condition_variable类成员,所以此类不支持复制构造函数也不支持赋值操作符(=)
 * */
template<class T>
class queue_safe{
private:  
	mutable std::mutex mut;   
	std::queue<T> data_queue;  
	std::condition_variable data_cond;  
public:  

	queue_safe(){

	}  
	queue_safe(queue_safe const& other)  
	{  
		std::lock_guard<std::mutex> lk(other.mut);  
		data_queue=other.data_queue;  
	}  
	
	void push(T new_value)//入队操作  
	{  
		std::lock_guard<std::mutex> lk(mut);  
		data_queue.push(new_value);  
		//data_cond.notify_all();  
		data_cond.notify_one();
	}  

	void wait_and_pop(T& value)//直到有元素可以删除为止  
	{  
		std::unique_lock<std::mutex> lk(mut);  
		data_cond.wait(lk,[this]{return !data_queue.empty();});  
		value=data_queue.front();  
		data_queue.pop();  
	}  

	std::shared_ptr<T> wait_and_pop()  
	{  
		std::unique_lock<std::mutex> lk(mut);  
		data_cond.wait(lk,[this]{return !data_queue.empty();});  
		std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));  
		data_queue.pop();  
		return res;  
	}  

	bool try_pop(T& value)//不管有没有队首元素直接返回  
	{  
		std::lock_guard<std::mutex> lk(mut);  
		if(data_queue.empty())  
			return false;  
		value=data_queue.front();  
		data_queue.pop();  
		return true;  
	}  

	std::shared_ptr<T> try_pop()  
	{  
		std::lock_guard<std::mutex> lk(mut);  
		if(data_queue.empty())  
			return std::shared_ptr<T>();  
		std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));  
		data_queue.pop();  
		return res;  
	}  
	bool empty() const  
	{  
		std::lock_guard<std::mutex> lk(mut);  
		return data_queue.empty();  
	}  
	int size() const {
		std::lock_guard<std::mutex> lk(mut);
		return data_queue.size();
	
	}
}; /* queue_safe */

#endif /* COMMON_SOURCE_CPP_THREADSAFE_QUEUE_H_ */