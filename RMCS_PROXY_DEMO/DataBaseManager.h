#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H
#include "CThread.h"
#include "common.h"
class DataBaseManager:public CThread
{
	//���Ǹ��߳���
	//����ά��һ������
	//run��������feedback��
	//�����뵽���غ�Զ�˵����ݿ�
	//�ṩһЩ�ӿ�
	//��feedback�������ܹ����������������
	//�����feedback�Ǹ�ʽ�ɶ������
	//������Ҫά������д���ݿ��������
	//һ�����ӱ��صģ�һ������Զ�̵�
	//��ȡ������Ľӿڣ��������ݿ��ʵ��д��
public:
	DataBaseManager(int st=DEFAULT_SLEEP_TIME):sleep_time(st){}
	~DataBaseManager(){

	}
	void run() override;
	void init() {
		this->start();
	}
	bool updateGroupFeedBackInDataBase(int type);//����̵߳��ã��ŵ�databaseManager�����queue,�и������жϵ�����״̬��һ��
	bool updateGroupInfoToDataBase();//����߳��ã��ŵ�databaseManager�����queue
	bool customerFeedBackQueue();//run�����е���
	bool customerGroupInfoQueue();//run�����е���
	bool flushDatabase(GroupfeedbackCustomStruct gfd,int type);//�������ݿ⣬���ò�ͬ��,custom�����е���

private:
	int sleep_time;


};

class DataBaseConnection{
	//������װ�˶����ݿ�����ӣ���ȡconnection����
	//init������������ݿ�����,�������
	//insert�������д��




};



#endif // !