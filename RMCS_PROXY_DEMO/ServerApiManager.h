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
	string ip; //ip��ַ
	int port; //�˿�
	bool connected;//Ĭ��Ϊ��
	SOCKET m_sock;
	vector<char> buf;
	vector<string> buf_vec;
	int buf_size;
	int sleepTime;
public:
	bool getCommandstrFromStream();//�����ж�ȡ����
	bool init();//��ʼ��
	bool connectToWebServer();//����Զ�����ݿ�
	ServerApiManager(string ip_, int port_, queue_safe<CommandGroupStruct>& gcmd_queue, int buf_size_ = BUF_SIZE, int sleepTime_=DEFAULT_SLEEP_TIME);
	~ServerApiManager();//�ͷ�����
	void setIpAndPort(string ip_,int port);//���ö˿�
	bool isconnected();//�Ƿ�������
	virtual void run() override;
};

#endif