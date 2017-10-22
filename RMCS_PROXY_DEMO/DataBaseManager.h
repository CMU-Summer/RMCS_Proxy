#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H
#include "CThread.h"
#include "common.h"
class DataBaseManager:public CThread
{
	//这是个线程类
	//自身维护一个队列
	//run里面消耗feedback，
	//并放入到本地和远端的数据库
	//提供一些接口
	//让feedback消费者能够放入自身队列钟来
	//放入的feedback是格式可读化后的
	//这里面要维护两个写数据库的连接类
	//一个连接本地的，一个连接远程的
	//调取连接类的接口，进行数据库的实际写入
public:
	DataBaseManager(int st=DEFAULT_SLEEP_TIME):sleep_time(st){}
	~DataBaseManager(){

	}
	void run() override;
	void init() {
		this->start();
	}
	bool updateGroupFeedBackInDataBase(int type);//别的线程调用，放到databaseManager里面的queue,有个区别判断的连接状态不一样
	bool updateGroupInfoToDataBase();//别的线程用，放到databaseManager里面的queue
	bool customerFeedBackQueue();//run函数中调用
	bool customerGroupInfoQueue();//run函数中调用
	bool flushDatabase(GroupfeedbackCustomStruct gfd,int type);//更新数据库，调用不同的,custom函数中调用

private:
	int sleep_time;


};

class DataBaseConnection{
	//这个类封装了对数据库的连接，获取connection对象
	//init函数解决和数据库连接,返回真假
	//insert函数解决写入




};



#endif // !