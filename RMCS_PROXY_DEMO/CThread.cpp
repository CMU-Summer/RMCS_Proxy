#include <iostream>
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include<stdexcept>
#include "CThread.h"
CThread::CThread(){
	this->t = NULL;
}  

CThread::~CThread(){
	this->stop();
	if( this-> t ){
		delete t;
	}
}  
void CThread::start(){
	if(!this-> t ){
		this->t = new std::thread(&CThread::run,this);
		//this->t->detach() ; //detach the thread, run continue
	}
}  //start thread

std::thread::native_handle_type  CThread::get_native_handleType(){
	return t->native_handle();


} //
void CThread::stop(){
	try
	{
		if(!this-> t ){
			//宏定义，暂时不知道怎么写
		}

	}
	catch (std::exception e)
	{
		std::cout<<e.what()<<std::endl;
	}
	

}     //stop thread
void CThread::timeSleeps(){



} //set sleep time
void CThread::join(){
	if(t){
		if (t->joinable()) {
			t->join();
			printf("thread  joins \n");
		}
		else {
			printf("thread cannot join\n");
		
		}
	}
}      //
bool CThread::swap(CThread* t1 ){
	if(t && t1->getThread()){
		t->swap(*(t1->getThread()));
	}
	return false;
}	  //swap thread 
std::thread::id CThread::get_id(){return t->get_id();} //thread id
bool CThread::joinable(){return t->joinable();} //
std::thread* CThread::getThread(){
	return this->t;


}