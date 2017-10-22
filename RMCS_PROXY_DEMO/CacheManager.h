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
	������Ǹ��߳��࣬����ά���������У�
	��feedback�����߷ŵ�feedback�����������
	lookup����»��������familyAndNames map
	�ṩ�����е��б����map�Ĳ����ӿ�
	*/
private:

	bool isConnect;//�Ƿ��Ѿ�����
	queue_safe<map<string,vector<string>>> family_name_queue;
	queue_safe<vector<GroupStruct>> group_struct_queue;//��Ҫ����ˢ
	queue_safe<GroupfeedbackCustomStruct> group_feedback_queue;
	ConfigManager& cfgManager;
	bool canUseRedis;

	std::condition_variable data_cond;  
public:
	bool stop_flag; // stop signal
	void CacheManager::forceSetEndTime(bool flag); // ǿ���������һ����¼��endTime
	void CacheManager::updateModuleInfo(string familyName, string name, ModuleInfo mod_, INT64 endTime);
	void CacheManager::insertModuleInfo(string familyName, string name, INT64 timestamp);
	ModuleInfo CacheManager::getLastModuleInfo(string familyName, string name);
	CacheManager(ConfigManager& cofManager, int sleep_time = DEFAULT_SLEEP_TIME);
	~CacheManager();
	void initCacheManager();//���ӣ����̵߳ȵȲ���
	CacheConnection cacheConnect;
	vector<FamilyStruct> getFamilyInCache();//��ȡcache�е�family
	vector<GroupStruct> getGroupInCache();//��ȡcache�е�list
	bool updateCacheFamilyAndItsNames(map<string,vector<string>> familyMap);
	//�����ڲ��������
	//����������ϵģ���ô���ص���true
	//�������ͨ���ڱ���̵߳�
	//����
	bool updateCacheGroupStateList(vector<GroupStruct> groupStructVec);
	//�������
	//��
	bool isConnected();//�Ƿ����������ϵ�
	void customfamilyMap();//���Ķ��������familyMap����
	void customGroupStateMap(); // ���Ķ��������groupStructVec����
	void customGroupFeedBack(); //���Ķ��������groupFeedBack
	void setIpAndPort(string ip_,string port_);//��������
	void run() override;//�������к���
	bool reconnect();//��������

	bool flushCacheAndItNameList(const string& key ,vector<string>& list);//�Ѷ����Ž���������
	bool updateGroupFeedBack(GroupfeedbackCustomStruct gfd);//���»���feedbacklist,ֻ������
	string getGroupCommandJsonStrFromCache();//��ȡһ������'
	bool updateFixedGroupToCache(vector<GroupStruct>& fixedGroupVec);//
	int sleep_time;
private:
	vector<FamilyStruct> getFamilyInCache_pri(string des);//��ȡ�е�cache�е�family
	vector<GroupStruct> getGroupInCache_pri(string des);//��ȡcache�е�list
	bool flushCacheGroupState(GroupStruct gst);//ˢ�»��������groupstrut
	bool flushCacheGroupFeedBackList(GroupfeedbackCustomStruct gfd);//ˢ��Զ�˻��������groupFeedback
	
	void flushFixedGroupNameToCache(string gName);
};


#endif // !