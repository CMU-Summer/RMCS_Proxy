#ifndef	FEEDBACKCUSTOMER_H
#define FEEDBACKCUSTOMER_H
#include "queue_safe.h"
#include "FeedBackManager.h"
#include "CacheManager.h"
#include "DataBaseManager.h"
#include "LookUpManager.h"
#include "CThread.h"
#include "common.h"
#define  LOCAL 1
#define  WEB 2
class FeedBackCustomer:public CThread{
	//这个类就是所有feedback的消费者
	//是一个线程类
	//可读化feedback已经在feedbackmanager里面定义好了
	//可读化feedback里面已经有一个具体的group信息，不用担心无法确定是那个group的feedback
	//调用缓存接口，写入到服务器缓存里面去
private: 
	
	queue_safe<GroupfeedbackCustomStruct>& gfd_quee;
	CacheManager& cacheManager;
	DataBaseManager& dbManager;
	int sleeptime;
public:
	bool stop_flag; // stop signal
	FeedBackCustomer(queue_safe<GroupfeedbackCustomStruct>& q,CacheManager& cacheManager_,DataBaseManager dbManager_,int sleeptime_=DEFAULT_SLEEP_TIME):
	cacheManager(cacheManager_),
	dbManager(dbManager_),
	sleeptime(sleeptime_),
	gfd_quee(q)
	{


	}
	  void run() override; //重载函数
	  void init(); //初始化

	  bool insertCache(GroupfeedbackCustomStruct& gfd);//放进缓存里面
	  bool insertDb(int type,GroupfeedbackCustomStruct& gfd);//放进数据库里面
	  bool customGfd();//消费
	  void printGroupFeedBack(GroupfeedbackCustomStruct& gfd){
		
		  printf("-------------\n");
		
		  printf("feedback from:\n");
		
		  printf("size:\n");
		;
		  printf("+++++++++++++++++++++++++\n");
		  for(int i=0;i<gfd.moduleFeedBackVec.size();i++){
				printf("module[%d]'s:（%d,%d,%d)\n",i ,gfd.moduleFeedBackVec.at(i)->led_field.led_R, gfd.moduleFeedBackVec.at(i)->led_field.led_G, gfd.moduleFeedBackVec.at(i)->led_field.led_B);
		  }
		  printf("+++++++++++++++++++++++++\n");
		
	  }
};



#endif