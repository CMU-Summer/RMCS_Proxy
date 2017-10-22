#ifndef CACHEMANAGER_H
#define CACHEMANAGER_H
#include "CThread.h"
#include "common.h"
#include <string>
#include "queue_safe.h"
#include <map>
#include <vector>
#include <mutex>
#include <condition_variable>
#include "ConfigManager.h"

using namespace std;
class CacheManager:public CThread
{
	/**
	这个类是个线程类，自身维护两个队列，
	让feedback消费者放到feedback待存队列里面
	lookup会更新缓存里面的familyAndNames map
	提供缓存中的列表或者map的操作接口
	*/
private:

	bool isConnect;//是否已经连接
	queue_safe<map<string,vector<string>>> family_name_queue;
	queue_safe<vector<GroupStruct>> group_struct_queue;//主要用来刷
	queue_safe<GroupfeedbackCustomStruct> group_feedback_queue;
	ConfigManager& cfgManager;
	bool canUseRedis;

	std::condition_variable data_cond;  
public:
	bool stop_flag; // stop signal
	void CacheManager::forceSetEndTime(bool flag); // 强行设置最后一条记录的endTime
	void CacheManager::updateModuleInfo(string familyName, string name, ModuleInfo mod_, INT64 endTime);
	void CacheManager::insertModuleInfo(string familyName, string name, INT64 timestamp);
	ModuleInfo CacheManager::getLastModuleInfo(string familyName, string name);
	CacheManager(ConfigManager& cofManager, int sleep_time = DEFAULT_SLEEP_TIME);
	~CacheManager();
	void initCacheManager();//链接，跑线程等等操作
	CacheConnection cacheConnect;
	vector<FamilyStruct> getFamilyInCache();//获取cache中的family
	vector<GroupStruct> getGroupInCache();//获取cache中的list
	bool updateCacheFamilyAndItsNames(map<string,vector<string>> familyMap);
	//函数内部放入队列
	//如果是连接上的，那么返回的是true
	//这个函数通常在别的线程掉
	//加锁
	bool updateCacheGroupStateList(vector<GroupStruct> groupStructVec);
	//放入队列
	//锁
	bool isConnected();//是否正在连接上的
	void customfamilyMap();//消耗队列里面的familyMap，锁
	void customGroupStateMap(); // 消耗队列里面的groupStructVec，锁
	void customGroupFeedBack(); //消耗队列里面的groupFeedBack
	void setIpAndPort(string ip_,string port_);//重新设置
	void run() override;//重置运行函数
	bool reconnect();//重新连接

	bool flushCacheAndItNameList(const string& key ,vector<string>& list);//把东西放进缓存里面
	bool updateGroupFeedBack(GroupfeedbackCustomStruct gfd);//更新缓存feedbacklist,只增不减
	string getGroupCommandJsonStrFromCache();//获取一条命令'
	bool updateFixedGroupToCache(vector<GroupStruct>& fixedGroupVec);//
	int sleep_time;
private:
	vector<FamilyStruct> getFamilyInCache_pri(string des);//获取中的cache中的family
	vector<GroupStruct> getGroupInCache_pri(string des);//获取cache中的list
	bool flushCacheGroupState(GroupStruct gst);//刷新缓存里面的groupstrut
	bool flushCacheGroupFeedBackList(GroupfeedbackCustomStruct gfd);//刷新远端缓存里面的groupFeedback
	
	void flushFixedGroupNameToCache(string gName);
};


#endif // !