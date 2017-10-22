#ifndef SERVERAPIMANAGER_H
#define SERVERAPIMANAGER_H
#include "common.h"

#include <string>  
#include <iostream>
#include "queue_safe.h"
#include "CThread.h"
#include <stdio.h>

#define BUF_SIZE 1024
class ServerApiManager:public CThread{
	
private:
	queue_safe<CommandGroupStruct>& group_command_queue;
	string ip; //ip地址
	int port; //端口
	bool connected;//默认为假
	SOCKET m_sock;
	vector<char> buf;
	vector<string> buf_vec;
	int buf_size;
	int sleepTime;
public:
	bool getCommandstrFromStream();//从流中读取命令
	bool init();//初始化
	bool connectToWebServer();//连接远程数据库
	ServerApiManager(string ip_, int port_, queue_safe<CommandGroupStruct>& gcmd_queue, int buf_size_ = BUF_SIZE, int sleepTime_=DEFAULT_SLEEP_TIME);
	~ServerApiManager();//释放连接
	void setIpAndPort(string ip_,int port);//设置端口
	bool isconnected();//是否连接上
	virtual void run() override;
};

#endif