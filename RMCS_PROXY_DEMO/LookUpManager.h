#ifndef	LOOKUPMANAGER_H
#define LOOKUPMANAGER_H
#include "CThread.h"
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <chrono>
#include <lookup.hpp>
#include <group.hpp>
#include <group_feedback.hpp>
#include <group_command.hpp>
#include <feedback.hpp>
#include <command.hpp>
#include "CacheManager.h"
#include "queue_safe.h"
#include "FeedBackManager.h"
#include "ConfigManager.h"
#include "common.h"
#define FIXED_TYPE 0
#define CACHE_TYPE 1
using namespace std;
using namespace hebi;
class LookUpManager:public CThread{
	//这里是lLookUpManager管理类
	//它是个线程类
	//需要周期性的更新family和names
	//更新缓存的family and name map
	//并且周期性的从缓存中读取用户定义的group
	//更新group的连接状态
	//需要缓冲管理类
	//需要定义缓存里面的gruopStateMap
	/*
	*/

private:
	
	hebi::Lookup& lookup; //hebi接口对象
	int default_frequency ;
	queue_safe<GroupfeedbackCustomStruct>& groupFeedbackQueue;//收到的异步groupfeedback都应该放到这里

	//group会增加和减少,会不会影响什么？估计不会
	map<string, shared_ptr<hebi::Group>>& cacheGroupMap;//需要更新的map,从缓存里面取，如果key没有就取一下addFh,有就不管
	map<string, shared_ptr<hebi::Group>>& fixedGroupMap;
	vector<FeedBackManager> feedbackManagerVec;
	vector<GroupStruct> fixedGroup;
	CacheManager& cacheManager;
	ConfigManager& configManager;
	int sleep_time; //休眠时间，取默认值
	bool fixedAdded;
	

	void updateFamilyAndNamesMap(map<string,vector<string>> newFandNMap);//更新缓存中的map,指针在函数内进行销毁
	map<string,vector<string>> getNewestMapFromHibi();//获得最新的family map的引用,返回处的函数中进行销毁
	bool isFirstRun;//是否第一次运行，是的话为true，否则为false,初始化为false,第一次run的时候不去读，等sleep时间,然后getEntryList

	vector<FamilyStruct> getFamilyListFromCache(); //从缓存中获取当前所有family信息
	vector<GroupStruct> getGroupListFromCache();//从缓存里获取当前用户定义的group的信息,到map去查
	void updateGroupConncetState(vector<GroupStruct> groupInCache,int default_timeout=0);//刷新group的连接状态,这里面的
	bool updateGroupsStateInCache(vector<GroupStruct> groupStrut);//这个放到cacheManager的队列里面去，能放进去就是true
	vector<GroupStruct> getGroupsStateFromHeibi(vector<GroupStruct> groupVec,int default_int);//从heibi里面获得cahce里面定义的group的连接状态
	void changecacheGroupMap(vector<GroupStruct> groupStrut);//修改map，删除掉不存在的
	

	vector<GroupStruct> getGroupListFromConfig();//从配置文件里面获取
	
	void updateModuleLoad(vector<FamilyStruct> fst_vec); //更新维护模块开关机信息
	void addHandlerFromGroups(vector<GroupStruct> gst_vec);//为fixed和缓存的group加处理函数
	void addHandlerForOneGroup(vector<string>* &familyVec,vector<string>* &nameVec,string groupName,int type);//添加一个group的
	void getFamilyAndNamesFromGroupStruct(GroupStruct& thisGroup,vector<string>* &familysVec,vector<string>* &namesVec);
	void showGroupFeedBackInfo(const hebi::GroupFeedback* group_fbk);//展示group的信息

public:
	bool stop_flag; // stop signal
	LookUpManager(
		CacheManager& cacheManager,
		queue_safe<GroupfeedbackCustomStruct>& groupFeedbackQueue,
		hebi::Lookup& lookup_,
		ConfigManager& configManager_,
		map<string, shared_ptr<hebi::Group>>& cacheGroupMap_,
		map<string, shared_ptr<hebi::Group>>& fixedGroupMap_,
		int sleep_time_=DEFAULT_SLEEP_TIME,
		int default_frequency_ = DEFAULT_FD_FREQUENCY
		);
	//需要什么参数,缓存管理类，简单，不需要信息沟通,没必要生产者消费者模型,
	//传进来缓存管理对象，队列对象，
	//引用要在初始化列表中赋值
	//lookup在里面创建
	~LookUpManager();
	void LookUpManager::forceSetEndTime();

	void run() override; // override run function,to find 
	void init() ;//init the lookupManager,比如跑起来
	hebi::Lookup& getLookUp(){return this->lookup;}

	static const int  DEAULT_SLEEP_TIME=50; //默认的休眠时间


}; 

#endif