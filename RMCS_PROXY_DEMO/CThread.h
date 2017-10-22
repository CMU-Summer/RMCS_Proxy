#ifndef THREAD_CUSTOM_H
#define THREAD_CUSTOM_H

#include <thread>
#include <iostream>  
class CThread
{
public:  
	CThread();  
	virtual ~CThread();  
	void start();  //start thread
	void stop();     //stop thread
	void timeSleeps(); //set sleep time
	void join();      //
	bool swap(CThread* t );	  //swap thread 
	std::thread::id get_id(); //thread id
	bool joinable(); //
	std::thread::native_handle_type get_native_handleType();
	std::thread* getThread();
private:  
	std::thread* t; //thread object
	
protected:
	virtual void run()=0; //run function
};




#endif