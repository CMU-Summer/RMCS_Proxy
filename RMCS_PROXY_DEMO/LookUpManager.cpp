#include "LookUpManager.h"
#include "src/lookup.hpp"
#include "ConfigManager.h"
#include "FeedBackManager.h"
#include <chrono>
#include <ctime>
#define D_S_T 50
using namespace std::chrono;
using namespace std;
using namespace hebi;
LookUpManager::LookUpManager(CacheManager& cacheManager_,
							 queue_safe<GroupfeedbackCustomStruct>& groupFeedbackQueue_,
							 Lookup& lookup_,
							 ConfigManager& configManager_,
							 map<string, shared_ptr<hebi::Group>>& cacheGroupMap_,
							 map<string, shared_ptr<hebi::Group>>& fixedGroupMap_,
							 int sleep_time_,
							 int default_frequency_
)
	:cacheManager(cacheManager_)
	,groupFeedbackQueue(groupFeedbackQueue_)
	,lookup(lookup_)
	,sleep_time(sleep_time_)
	,configManager(configManager_)
	,fixedGroup()
	,cacheGroupMap(cacheGroupMap_)
	,feedbackManagerVec()
	,fixedAdded(false)
	,default_frequency(default_frequency_)
	,fixedGroupMap(fixedGroupMap_)
{
	//初始化
	//获取值



}
//需要什么参数,缓存管理类，简单，不需要信息沟通,没必要生产者消费者模型,
//传进来缓存管理对象，队列对象，
//引用要在初始化列表中赋值
//lookup在里面创建
LookUpManager::~LookUpManager(){
	
	// 进行异常处理,将所有不完整的记录的结束时间全部置位
	this->cacheManager.forceSetEndTime(false);


}
void LookUpManager::run() {
	//逻辑运行的地方
	while(true){

		if (stop_flag) return; // if stop signal reveived

		//再次确认，抽取cache里面的group，更新状态；更新cahce里面的family和name的关系表
		vector<GroupStruct> gst_vec = this->getGroupListFromCache();//取出来

		vector<FamilyStruct> fst_vec = this->getFamilyListFromCache(); 

		//0.更新维护模块开关机信息
		this->updateModuleLoad(fst_vec);

		//1.更新维护的cacheGroupList列表,删除Redis中已被删除的group
		this->changecacheGroupMap(gst_vec);


		//2.给新增的group添加handler
		this->addHandlerFromGroups(gst_vec);

		
		//3.添加新连接入的模块信息
		this->updateFamilyAndNamesMap(this->getNewestMapFromHibi());


		//4.刷新group里面的module的连接状态
		this->updateGroupConncetState(gst_vec);

		


		//5。线程暂停
		if(this->sleep_time>0){
			this_thread::sleep_for(std::chrono::milliseconds(this->sleep_time));
		}
		printf("LOOKUPMANAGER_THREAD: lookupManager thread next executed\n");
	}


} // override run function,to find 

void LookUpManager::forceSetEndTime() {
	this->cacheManager.forceSetEndTime(false);
}

void LookUpManager::init() {
	stop_flag = false;
	
	this->cacheManager.forceSetEndTime(true);

	//获取
	vector<GroupStruct> grpList=this->getGroupListFromConfig();


	this->fixedGroup.assign(grpList.begin(),grpList.end()); 
	

	vector<GroupStruct> groupStructVec;
	for (int i = 0;i < fixedGroup.size();i++) {
		GroupStruct groupStruct;
		string groupName = fixedGroup.at(i).getName();
		groupName = groupName + "_fix";
		fixedGroup.at(i).setName(groupName);
		groupStruct.setName(groupName);
		vector<FamilyStruct*> familyList = fixedGroup.at(i).familyList;
		for (int j = 0;j < familyList.size();j++) {
			FamilyStruct* family = new FamilyStruct();
			family->setName(familyList.at(j)->getName());
			vector<NameStruct*> nameList = familyList.at(j)->nameList;
			for (int k = 0;k < nameList.size();k++) {
				NameStruct* name = new NameStruct();
				name->name = nameList.at(k)->name;
				name->connected = nameList.at(k)->connected;
				family->nameList.push_back(name);
			}
			groupStruct.familyList.push_back(family);
		}
		groupStructVec.push_back(groupStruct);
	}
	this->cacheManager.updateCacheGroupStateList(groupStructVec);
	//这里加一行
	this->cacheManager.updateFixedGroupToCache(this->fixedGroup);


	
	this->start();//启动线程,还有其他的操作？

}//init the lookupManager,比如跑起来

void LookUpManager::updateFamilyAndNamesMap(map<string,vector<string>> newFandNMap){
	
	this->cacheManager.updateCacheFamilyAndItsNames(newFandNMap);
	
}//更新缓存中的map,指针在函数内进行销毁
map<string,vector<string>> LookUpManager::getNewestMapFromHibi(){
	//从hebi里面获取
	map<string,vector<string>> fMap;
	Lookup lookup;
	shared_ptr<hebi::Lookup::EntryList> entrylist = lookup.getEntryList();
	if (!entrylist) {
		printf("LOOKUPMANAGER_THREAD: entrylist is null !!\n");
		return fMap;
	}
	for(int i=0; i<entrylist->size();i++){
		auto entry = entrylist->getEntry(i);
		if(fMap.count(entry.family_)){
			//包含，就放进去
		
			(fMap[entry.family_]).push_back(entry.name_);

		}else {
			//不包含
			fMap[entry.family_]=vector<string>();//
			fMap[entry.family_].push_back(entry.name_);

		}

	}
	return fMap;
}//获得最新的family map的值


vector<GroupStruct> LookUpManager::getGroupListFromCache(){
	return this->cacheManager.getGroupInCache();

}//从缓存里获取当前用户定义的group的信息,到map去查

vector<FamilyStruct> LookUpManager::getFamilyListFromCache() {
	return this->cacheManager.getFamilyInCache();

}//从缓存里获取当前缓存中的family的信息


void LookUpManager::updateGroupConncetState(vector<GroupStruct> groupInCache,int default_timeout){
	//根据这些group，去判断是否连接了
	if( groupInCache.size()>0 && this->updateGroupsStateInCache(getGroupsStateFromHeibi(groupInCache,default_timeout))){
		//成功刷新缓存
		
		printf("LOOKUPMANAGER_THREAD: group connection status flush successfully\n");

	}else {
		if(cacheManager.isConnected()){
			//连接上了，就抱个错
		}else{
			//没有连接上，既然是刷新状态，没必要干什么了
	
			printf("LOOKUPMANAGER_THREAD: disconnected,can not flush\n");
		}
	}

}//刷新group的连接状态,这里面的


bool LookUpManager::updateGroupsStateInCache(vector<GroupStruct> groupStrut){
	return this->cacheManager.updateCacheGroupStateList(groupStrut);
}//这个放到cacheManager的队列里面去，能放进去就是true


void LookUpManager::updateModuleLoad(vector<FamilyStruct> fst_vec) {
	string familyName;
	string name;
	for (int i = 0;i < fst_vec.size();i++) {
		FamilyStruct family_st = fst_vec.at(i);
		familyName = family_st.getName();
		for (int j = 0;j < family_st.nameList.size();j++) {
			name = family_st.nameList.at(j)->name;
			// =.=
			Lookup lookup;
			shared_ptr<Group> grp = lookup.getGroupFromNames({ familyName } ,{ name }, DEAULT_SLEEP_TIME);
			ModuleInfo mod = this->cacheManager.getLastModuleInfo(familyName, name);
			if (grp) {
				if (mod.startTime != -1 && mod.endTime == -1) {
					continue; // 模块已经开启，并且已有记录
				}
				else {
					// 已有模块或者新模块刚开启，写入新纪录
					auto time_now = chrono::system_clock::now();
					auto duration_in_ms = chrono::duration_cast<chrono::milliseconds>(time_now.time_since_epoch());
					INT64 timestamp = (INT64)duration_in_ms.count();
					this->cacheManager.insertModuleInfo(familyName, name, timestamp);
				}

			}
			else {
				if (mod.startTime == -1) {
					continue; // 出现异常，但不影响整体记录
				}
				else {
					if (mod.endTime == -1) {
						// 更新已开启模块的关闭时间
						auto time_now = chrono::system_clock::now();
						auto duration_in_ms = chrono::duration_cast<chrono::milliseconds>(time_now.time_since_epoch());
						INT64 endTime = (INT64)duration_in_ms.count();
						this->cacheManager.updateModuleInfo(familyName, name, mod, endTime);
					}
					else {
						continue; //模块已关闭，并且已被成功记录
					}
				}

			}
		}

	}
	// Release the resource
	for (int i = 0;i < fst_vec.size();i++) {
		fst_vec.at(i).freeStruct();
	}
}



vector<GroupStruct> LookUpManager::getGroupsStateFromHeibi(vector<GroupStruct> groupVec,int default_int){
	for(int i=0;i<groupVec.size();i++){
		for(int j=0;j<groupVec.at(i).getFamilyList().size();j++){
			//每个family
			string familyName=groupVec.at(i).getFamilyList().at(j)->getName();
			//遍历Map
			vector<NameStruct*>& nameListMap=groupVec.at(i).getFamilyList().at(j)->getNameList();
			vector<NameStruct*>::iterator it;
			for(it=nameListMap.begin();it!=nameListMap.end();it++){
				// =.= 
				Lookup lookup;
				shared_ptr<Group> grp = lookup.getGroupFromNames({ familyName }, { (*it)->name },DEAULT_SLEEP_TIME);

				if(grp){
					(*it)->connected=true;//更新状态为true
				}
				else {
					(*it)->connected = false;//更新状态为false
				}

			}	
		}
	}	
	return groupVec;//更新完返回


}//从heibi里面获得cahce里面定义的group的连接状态


vector<GroupStruct> LookUpManager::getGroupListFromConfig(){
	//获取从配置文件读的group
	 return this->configManager.getGroupList();




}//从配置文件里面获取
void LookUpManager::addHandlerFromGroups(vector<GroupStruct> gstVec){
	//给fixedgroup添加

	
	for(int i=0;i<gstVec.size();i++){
		printf("LOOKUPMANAGER_THREAD: add FeedBack Handle to Cache Group\n");
		if(this->cacheGroupMap.count(gstVec.at(i).getName())){
			//如果包含什么也不做
			printf("LOOKUPMANAGER_THREAD: this group is already handled!\n");
		}else {
			//没有，这个group还没有一个feedbackManager来处理
			vector<string>* familyVec=new vector<string>(),*nameVec = new vector<string>();
			//获取names和familys
			this->getFamilyAndNamesFromGroupStruct(gstVec.at(i),familyVec,nameVec);
			//添加feedbackManager
			string gname= gstVec.at(i).getName();
			this->addHandlerForOneGroup(familyVec,nameVec,gname,CACHE_TYPE);
			//释放指针
			delete familyVec,nameVec;
			//吧这个字符串对应的cacheMap里面去
			
		}

	}


}//为fixed和缓存的group加处理函数

void LookUpManager::addHandlerForOneGroup(vector<string>* &familyVec,vector<string>* &nameVec,string groupName,int type){
	//传进来的，根据group的名字，如果cache里面有，就不加了
	if (this->cacheGroupMap.count(groupName) > 0)return;
	else if (this->fixedGroupMap.count(groupName) > 0)return;
	
	
	FeedBackManager* fdbManager=new FeedBackManager(this->groupFeedbackQueue); 
	Lookup lookup;
	shared_ptr<Group> grp= lookup.getGroupFromNames(*familyVec, *nameVec,DEAULT_SLEEP_TIME);
	if (!grp){
		printf("LOOKUPMANAGER_THREAD: group from hebi is null!s\n");
		return;//null不用加
	}
	LookUpManager* this_ = this;
	string* groupName_ = new string(groupName);
	grp->setFeedbackFrequencyHz(this->default_frequency);
	grp->addFeedbackHandler(
		[fdbManager, groupName_, &this_](const hebi::GroupFeedback& group_fbk)->void
	{
		//this_->showGroupFeedBackInfo(&group_fbk);
		GroupfeedbackCustomStruct gfb_custom = fdbManager->toGroupFbCustomStruct(&group_fbk, groupName_->data());

		fdbManager->putToQueue(gfb_custom);
	});
	if (type == FIXED_TYPE) {
		this->fixedGroupMap.insert(map<string, shared_ptr<Group>>::value_type(groupName, std::move(grp))); //保留内存
	}
	else {
		this->cacheGroupMap.insert(map<string, shared_ptr<Group>>::value_type(groupName, std::move(grp)));//保留内存
		this->feedbackManagerVec.push_back(*fdbManager); //放进vec里面管理
	}
	
}
void LookUpManager::getFamilyAndNamesFromGroupStruct(GroupStruct& thisGroup,vector<string>* &familysVec,vector<string>* &namesVec){
	for(int j=0;j<thisGroup.getFamilyList().size();j++){
		familysVec->insert(familysVec->end(),thisGroup.getFamilyList().at(j)->getName());
		vector<NameStruct*>& nameListMap=thisGroup.getFamilyList().at(j)->getNameList();
		vector<NameStruct*>::iterator it;
		for(it=nameListMap.begin();it!=nameListMap.end();it++){
			namesVec->insert(namesVec->end(),(*it)->name);
		}
	}

}
void LookUpManager::showGroupFeedBackInfo(const GroupFeedback* group_fbk) {
	printf("LOOKUPMANAGER_THREAD: [-------GroupFeedBack------]\n[size:%d]\n", group_fbk->size());
	for (int i = 0; i < group_fbk->size(); i++) {
		printf("LOOKUPMANAGER_THREAD: module[%d] feedback info:", i);
		printf("voltage %f", (*group_fbk)[i].voltage().get());
		printf("Motor current %f", (*group_fbk)[i].actuator().motorCurrent().get());
		printf("position %f", (*group_fbk)[i].actuator().position().get());
		printf("velocity %f", (*group_fbk)[i].actuator().velocity().get());
		printf("torque %f\n", (*group_fbk)[i].actuator().effort().get());
	}
	printf("LOOKUPMANAGER_THREAD: [-------END------]\n");



}


void LookUpManager::changecacheGroupMap(vector<GroupStruct> groupStruts) {
	map<string, string> cMap;
	//构建一个Map
	for (int i = 0; i < groupStruts.size();i++) {
		string a = groupStruts.at(i).getName();
		cMap[a] = a;
	}
	//遍历保存的老的cacheMap
	map<string, shared_ptr<Group>>::iterator it;
	vector<string> groupsNeedtoDelete;
	for (it = this->cacheGroupMap.begin(); it != this->cacheGroupMap.end();it++) {
		if (cMap.count(it->first)<=0) {
			//最新的cache的group里面，不包含老的group，那就需要删除
			//放在一个数组里面
			groupsNeedtoDelete.push_back(it->first);
			printf("LOOKUPMANAGER_THREAD: this group need to be deleted :%s\n",it->first.data());
		}
	
	}
	for (int i = 0; i < groupsNeedtoDelete.size();i++) {
		
		this->cacheGroupMap.erase(groupsNeedtoDelete.at(i));//删除掉
	}

}
