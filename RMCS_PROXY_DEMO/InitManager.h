#ifndef INITMANAGER_H
#define INITMANAGER_H

#include "queue_safe.h"
#include "LookUpManager.h"
#include "CacheManager.h"
#include "ConfigManager.h"
#include "CommandCustomer.h"
#include "FeedBackCustomer.h"
#include "FeedBackManager.h"
#include <src/lookup.hpp>
#include <thread>
#include "common.h"
#include <chrono>
#include <stdlib.h>
class InitManager
	/*
	�������һ��ʼ��ʼ����
	��Ҫ�������ļ���ȡһϵ�в���
	���籾�����ݿ�/web���ݿ��ַ
		Ĭ����Ҫ���ӵ�group
	��ʼ������manager/customer�����߳������������������ʼ���������в����Ļ����Ͳ����ֶ�start��
	�ݶ�
	*/


{
public:

	InitManager() {
	
	}
	static int main(){

		printf("--------init queue!--------\n");
		queue_safe<GroupfeedbackCustomStruct> gfd_queue;
		queue_safe<GroupStruct> gs_queue;
		queue_safe<CommandGroupStruct> cmd_queue;
		map<string, shared_ptr<hebi::Group>> cacheGroupMap_;
		map<string, shared_ptr<hebi::Group>> fixedGroupMap_;
		printf("--------init hebi lookup--------\n");
		hebi::Lookup lookup;
		//��������
		int sleepTime;
		int fd_hz;
		printf("--------init configManager--------\n");
		ConfigManager cfgManager("resource/config.xml");
		vector<ServerConfig> sCof = cfgManager.getServersConfig();
		vector<RedisCofig>  rCof = cfgManager.getRedisList();
		vector<DBconfig>  dbCof=cfgManager.getDbConfig();
		CacheManager* cacheMangerPtr=NULL;
		LookUpManager* lkManagerPtr = NULL;
//		CommandCustomer* cmdCusromPtr = NULL;
		FeedBackCustomer* fdCustomerPtr = NULL;

		if (cfgManager.getSleepTime() <= DEFAULT_SLEEP_TIME) {
			sleepTime = DEFAULT_SLEEP_TIME;

		}else {
			sleepTime = cfgManager.getSleepTime();
		
		}
		if (cfgManager.getFeedbackFrequency() >= DEFAULT_FD_FREQUENCY) {
			fd_hz = DEFAULT_FD_FREQUENCY;

		}
		else {
			fd_hz = cfgManager.getFeedbackFrequency();

		}
		//���л������
		printf("---------init cacheManager--------\n");
		CacheManager cacheManger(cfgManager, sleepTime);
		//��ʼ�����ݿ����
		printf("---------init databaseManager ---------\n");
		DataBaseManager db;
		printf("---------init lookupManager ---------\n");
		//��ʼ��lookup�߳�
		LookUpManager lkManager(cacheManger,gfd_queue,lookup,cfgManager,cacheGroupMap_,fixedGroupMap_,sleepTime,fd_hz);
//       ��ʼ������������
//       printf("---------init CommandCustomer ---------\n");
//		CommandCustomer cmdCusrom(cmd_queue,cfgManager,cacheGroupMap_,fixedGroupMap_, sleepTime);
		
        //��ʼ������������
		printf("---------init FeedBackCustomer ---------\n");
		FeedBackCustomer fdCustomer(gfd_queue,cacheManger,db, sleepTime);

		printf("---------run cache thread ---------\n");
		cacheManger.initCacheManager();//1
		printf("---------run lookupManager thread ---------\n");
		lkManager.init();//2
//		printf("---------run commandCustomer thread ---------\n");
//		cmdCusrom.init();
		printf("---------run feedbackCustomer thread ---------\n");
		fdCustomer.init();//3

		printf("--------- working!---------\n");

		while (true) {
			string command;
			cin.clear();
			cin.sync();
			cin >> command;
			if (command == "exit") { // if receive STOP_COMMAND
				//lkManager.forceSetEndTime();
				cacheManger.stop_flag = true;
				lkManager.stop_flag = true;
				fdCustomer.stop_flag = true;
				break;
			}
				
		}
		cacheManger.join();
		printf("---------cacheManger join!---------\n");
//		fdCustomer.join();
//		printf("---------fdCustomer join!---------\n");
		lkManager.join();
		
		printf("---------lkManager join!---------\n");
//		cmdCusrom.join();
//		printf("---------cmdCusrom join!---------\n");

		printf("---------RMCS All Threads Stopped!\n-----------");

		exit(1);
		
	}
	~InitManager() {
	
	}

private:

};


#endif