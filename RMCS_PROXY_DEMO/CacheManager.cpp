#include "CacheManager.h"
#include <mutex>
#include <condition_variable>
#include "ConfigManager.h"
#include <stdexcept>
CacheManager::CacheManager(ConfigManager& cofManager,int sleep_time_ ):cacheConnect(),canUseRedis(true),sleep_time(sleep_time_),cfgManager(cofManager){




}
CacheManager::~CacheManager(){}

void CacheManager::initCacheManager(){
	stop_flag = false;
	vector<RedisCofig> cogVec=this->cfgManager.getRedisList();
	if (cogVec.size() <= 0) {
		printf("CACHE_MANAGER_THREAD: no redis in config:%d\n");
		this->isConnect = false;
	}
	else {
		this->cacheConnect.setIpAndPort(cogVec.at(0).ip,cogVec.at(0).port);
		try
		{
			this->isConnect = this->cacheConnect.init();
			if (this->isConnect) {
				//去验证密码
				bool opt=this->cacheConnect.authPwd(cogVec.at(0).password);
				if (opt) {
					printf("CACHE_MANAGER_THREAD: redis auth successfully \n");
					this->isConnect = true;
				}
				else {
					printf("CACHE_MANAGER_THREAD: redis auth failed\n");
					this->isConnect = false;//验证没通过等于没连
				}
			
			}



			printf("CACHE_MANAGER_THREAD: redis connect status:%d\n", this->isConnect);
		}
		catch (exception* e)
		{
			printf("CACHE_MANAGER_THREAD: redis connect error.\n");
		}
	}

	this->start();//跑线程


}//链接，跑线程等等操作

vector<GroupStruct> CacheManager::getGroupInCache(){
	//别的线程里面
	//cout << "get user groups from cache" << endl;
	printf("LOOKUPMANAGER_THREAD: get user groups from cache\n");
	return this->getGroupInCache_pri(GET_LIST);

}//获取cache中的list,需要锁

vector<FamilyStruct> CacheManager::getFamilyInCache() {
	printf("LOOKUPMANAGER_THREAD: get families from cache\n");
	return this->getFamilyInCache_pri(GET_LIST);
	
}

bool CacheManager::updateCacheFamilyAndItsNames(map<string,vector<string>> familyMap){
	//已经在队列函数中处理了锁

	printf("LOOKUPMANAGER_THREAD: put family and its names into queue\n");
	if(this->cacheConnect.isConnected() == false)return false;
	else {
		this->family_name_queue.push(familyMap);
		return true;
	}

}
//函数内部放入队列
//如果是连接上的，那么返回的是true
//这个函数通常在别的线程掉

bool CacheManager::updateCacheGroupStateList(vector<GroupStruct> groupStructVec){
	//cout<<"put groups connect status into queue"<<endl;
	printf("LOOKUPMANAGER_THREAD: put groups connect status into queue\n");
	if(this->cacheConnect.isConnected() == false)return false;
	else {
		this->group_struct_queue.push(groupStructVec);
		return true;
	}
	return true;
}
//放入队列
//锁
bool CacheManager::isConnected(){return this->cacheConnect.isConnected();}//是否正在连接上的
void CacheManager::customfamilyMap(){
	int index = 0;
	int size = family_name_queue.size();
	while (index<size)
	{
		shared_ptr<map<string, vector<string>>> mapPtr = this->family_name_queue.try_pop();
		if (!mapPtr)return;//没有取到

		printf("CACHE_MANAGER_THREAD: get family and names from queue\n");
		map<string, vector<string>>::iterator it;
		for (it = mapPtr->begin(); it != mapPtr->end(); it++) {
			//放进family的集合里面去
			//缓存里面有family的set 和group 的 set 
			this->flushCacheAndItNameList(it->first, it->second);
		}
		index++;
	}

}//消耗队列里面的familyMap，锁
void CacheManager::customGroupStateMap(){
	//
	int conut = 0;
	int size = group_struct_queue.size();
	while (conut<size) {
		shared_ptr<vector<GroupStruct>> mapPtr = this->group_struct_queue.try_pop();
		if (!mapPtr)return;//没有取到需要消耗的groupVec


		printf("CACHE_MANAGER_THREAD: get group status from queue\n");
		vector<GroupStruct>::iterator it;
		for (it = mapPtr->begin(); it != mapPtr->end(); it++) {

			//放之前要先判断下，这个group还在不在缓存里面了，因为有可能被删掉
			char* keyTemp = new char[strlen(it->getName().data()) + 1];

			if (this->cacheConnect.isContainKey(strncpy(keyTemp, it->getName().data(), it->getName().length() + 1))) {
				this->flushCacheGroupState(*it);
			}
			else if (it->getName().size() > 4) {
				string postfix = it->getName().substr(it->getName().size() - 4);
				if (postfix.compare("_fix") == 0)
					this->flushCacheGroupState(*it);
			}
			delete[] keyTemp;

		}
		conut++;
	}

} // 消耗队列里面的groupStructVec，锁
void CacheManager::setIpAndPort(string ip_,string port_){
	this->cacheConnect.setIpAndPort(ip_, std::atoi(port_.data()));
}//重新设置
void CacheManager::run(){
	while (true)
	{
		if (stop_flag) { // if stop flag received
			this->forceSetEndTime(false);
			return;
		}
		//cout<<"lookupManager thread work"<<endl;
		printf("CACHE_MANAGER_THREAD: cache manager thread work\n");
		customfamilyMap();//消耗下队列
		customGroupStateMap();
		customGroupFeedBack();
		if(this->sleep_time>0){
			this_thread::sleep_for(std::chrono::milliseconds(this->sleep_time));
		}
		printf("CACHE_MANAGER_THREAD: cache manager is ready for next\n");
	}
}//重置运行函数

bool CacheManager::reconnect(){
	if(this->cacheConnect.isConnected()==false){
		return this->isConnect=this->cacheConnect.reconnect();
	}else
	{
		return this->isConnect = true;
		
	}
}//重新连接

vector<FamilyStruct> CacheManager::getFamilyInCache_pri(string des) {
	void* familyP = NULL;
	int arg_nums = 2;
	const char* args[] = { "smembers","family", };
	bool opt = this->cacheConnect.setCommndWithArgs(arg_nums, args, des, familyP);//返回的是向量

	if (opt == false) {
		printf("LOOKUPMANAGER_THREAD: can not get family list from cache\n");
		vector<FamilyStruct> emptyGroup;
		return emptyGroup;

	}
	else {
		printf("LOOKUPMANAGER_THREAD: get family list from cache\n");

	}
	vector<string>& f_strs = *(vector<string>*)familyP;
	vector<FamilyStruct> familyV;
	for (int i = 0;i<f_strs.size();i++) {
		FamilyStruct f;
		f.setName(f_strs.at(i).data()); //设置family_name
		//取str
		void* str = NULL;
		char* family_key_ = new char[strlen(f_strs.at(i).data()) + 1];
		int arg_nums_1 = 2;
		const char* args_1[] = { "smembers",strncpy(family_key_,f_strs.at(i).data(),f_strs.at(i).length() + 1) };
		bool opt = this->cacheConnect.setCommndWithArgs(arg_nums_1, args_1, GET_STR, str);

		if (opt == false) {
			printf("CACHEMANAGER_THREAD: can not get family %s from cache\n", f_strs.at(i).data());

		}
		else {
			printf("CACHEMANAGER_THREAD: get family %s from cache\n", f_strs.at(i).data());
			vector<string>& n_strs = *(vector<string>*)str;
			vector<NameStruct*> names_vec;
			for (int j = 0;j < n_strs.size();j++) {
				NameStruct* name_st = new NameStruct();
				name_st->name = n_strs.at(j);
				names_vec.push_back(name_st);
			}
			f.setNameList(names_vec);
			delete str;
			delete[] family_key_;
		}
		familyV.push_back(f);
	}
	delete familyP;
	printf("CACHE_MANAGER_THREAD: delete familyP\n");
	return familyV; //返回

}


vector<GroupStruct> CacheManager::getGroupInCache_pri(string des){
		//先取 group的list,再取str
		void* groupP=NULL;
		int arg_nums=2;
		const char* args[]={"smembers","group",};
		bool opt = this->cacheConnect.setCommndWithArgs(arg_nums,args,des,groupP);//返回的是向量
			
		if (opt == false) {
			printf("CACHE_MANAGER_THREAD: can not get group list from cache\n");
			vector<GroupStruct> nullGroup;
			return nullGroup;
		
		}
		else {
			printf("CACHE_MANAGER_THREAD: get group list from cache\n");
		
		}
		vector<string>& g_strs = *(vector<string>*)groupP;
		vector<GroupStruct> groupV;
		for(int i=0;i<g_strs.size();i++){
			GroupStruct g;
			//取str
			void* str=NULL;
			char* group_key_ = new char[strlen(g_strs.at(i).data())+1];
			int arg_nums_1=2;
			const char* args_1[]={"get",strncpy(group_key_,g_strs.at(i).data(),g_strs.at(i).length()+1)};
			this->cacheConnect.setCommndWithArgs(arg_nums_1,args_1,GET_STR,str);//str已经有了一个str了
			if (str) {
				string* a = (string*)str;
				g.DeSerialize(a->data());
				delete str;
				delete[] group_key_;	
				groupV.push_back(g);
			
			}
	}
		delete groupP;
		printf("CACHE_MANAGER_THREAD: delete groupP\n");
		return groupV; //返回

	


}//取数据
//调用缓存类，放进去
bool CacheManager::flushCacheAndItNameList(const string& key ,vector<string>& list){
	//两个set都要add 
	void* groupP;
	int arg_nums=list.size()+2;
	char* key_=new char[strlen( key.data())+1];

	const char** args=new const char*[arg_nums];
	args[0]="sadd";
	args[1]=strncpy(key_,key.data(),key.length()+1);
	for(int i=0;i<list.size();i++){
		args[2+i]=list.at(i).data();
		
	}
	bool opt= this->cacheConnect.setCommndWithArgs(arg_nums,args,ADD_VALUE_TO_SET,groupP);

	char* key__=new char[strlen( key.data())+1];
	const char* args_[]={"sadd","family",strncpy(key__,key.data(),key.length()+1)};//放到family set里面去
	int arg_nums_=3;
	void* intx;
	bool opt1= this->cacheConnect.setCommndWithArgs(arg_nums_,args_,ADD_VALUE_TO_SET,intx);
	delete intx, groupP;
	delete[] key_, key__;
	/*
	for (int i = 0; i<list.size(); i++) {
		delete[] args[2 + i];
	}*/
	if (opt == false) {
		printf("CACHE_MANAGER_THREAD: cache error:cannot flush some family in cache \n");
	}
	if (opt1 == false) {
		printf("CACHE_MANAGER_THREAD: cache error:cannot flush  family list in cache \n");
	}
	return opt==opt1==true;

	printf("CACHE_MANAGER_THREAD: send newest family and names map to cache\n");
	return true;
}
bool CacheManager::flushCacheGroupState(GroupStruct gst){
	//判断有key 再set str
	 char* key_=new char[strlen( gst.getName().data())+1];
	 gst.SetPropertys();//重置一下
	 string s= gst.Serialize();

	char* objStr=new char[strlen(s.data())+1];
	const char* args_[]={"set",strncpy(key_,gst.getName().data(),gst.getName().length()+1),strncpy(objStr,s.data(),s.length()+1)};//放到family set里面去
	int arg_num=3;
	void* res=NULL;
	bool opt=this->cacheConnect.setCommndWithArgs(arg_num,args_,SET_STR,res);
	if (opt == false) {
		printf("CACHE_MANAGER_THREAD: cache error:cannot flush some group state  in cache \n");
	}
	if (res != NULL)delete res;
	delete[] key_, objStr;
	gst.freeStruct();

	return opt;



}
bool CacheManager::flushCacheGroupFeedBackList(GroupfeedbackCustomStruct gfd){
	//判断有key 再set str
	string gfd_name_key=gfd.groupName+"_gfd";//group的fd默认加个后缀
	char* key_=new char[strlen(gfd_name_key.data())+1];
	gfd.SetPropertys();
	string s(gfd.Serialize());
	char* objStr=new char[strlen(s.data())+1];
	const char* args_[]={"rpush",strncpy(key_,gfd_name_key.data(),gfd_name_key.length()+1),strncpy(objStr,s.data(),s.length()+1)};//放到groupfdlist set里面去
	int arg_num=3;
	void* res=NULL;
	bool opt=this->cacheConnect.setCommndWithArgs(arg_num,args_,SET_STR,res);
	if(res!=NULL)delete res,
	delete[] key_, objStr;
	if (opt == false) {
		printf("CACHE_MANAGER_THREAD: cache error:cannot add some group 's feedback  in cache \n");
	}
	gfd.freeStruct();
	return opt;



}//刷新远端缓存里面的groupFeedback
bool CacheManager::updateGroupFeedBack(GroupfeedbackCustomStruct gfd){
	printf("FDBKCUSTOMER__THREAD: put group feed back into queue\n");
	//cout<<"put group feed back into queue"<<endl;
	if(this->cacheConnect.isConnected() == false)return false;
	else {
		this->group_feedback_queue.push(gfd);
		return true;
	}

}//更新缓存feedbacklist,只增不减
void CacheManager::customGroupFeedBack(){
	int count=0;
	int size = group_feedback_queue.size();
	while (count<size)
	{
		shared_ptr<GroupfeedbackCustomStruct> mapPtr = this->group_feedback_queue.try_pop();
		if (!mapPtr)return;//没有取到需要消耗的groupVec
		printf("CACHE_MANAGER_THREAD: get one feedback from queue\n");
		this->flushCacheGroupFeedBackList(*mapPtr);
		mapPtr = this->group_feedback_queue.try_pop();
		count++;
	}

}
string CacheManager::getGroupCommandJsonStrFromCache() {
	//调用命令获取字符串数组
	void* str;
	int argNum = 2;
	const char* a[] = {"lpop","command_c"};//命令都在command_c里面
	if (this->isConnect == false)return "";
	if (this->cacheConnect.setCommndWithArgs(2, a, GET_STR, str)) {
		
		string s((*(string*)str));
		delete str;
		return s;
	}
	else {
		//成功了 str就是一个string
		//获取失败了
		return "";
	}


}
bool CacheManager::updateFixedGroupToCache(vector<GroupStruct>& gVec ) {
	for (int i = 0; i < gVec.size();i++) {
		this->flushFixedGroupNameToCache(gVec.at(i).getName());
	}
	return true; // = = 
}
void  CacheManager::flushFixedGroupNameToCache(string gName) {
	char* key_ = new char[strlen(gName.data()) + 1];
	strncpy(key_, gName.data(), gName.length() + 1);
	const char* args[] = { "sadd","group", key_};
	int argNums = 3;
	void* res=NULL;
	this->cacheConnect.setCommndWithArgs(argNums,args, ADD_VALUE_TO_SET, res);
	if ((int*)res <= 0) {
		printf("CACHE_MANAGER_THREAD: cannot add fixed group to cache \n");
	}
	if (!res)delete res; //释放空间
	delete[] key_; //删除键
}
 //消耗队列里面的groupFeedBack

ModuleInfo CacheManager::getLastModuleInfo(string familyName, string name) {
	void* modP = NULL;
	int arg_nums = 4;

	string key = familyName + "_" + name;
	char* key_ = new char[strlen(key.data()) + 1];
	strncpy(key_, key.data(), key.length() + 1);
	const char* args[] = { "lrange", key_,"-1", "-1" };

	bool opt = this->cacheConnect.setCommndWithArgs(arg_nums, args, GET_LIST, modP);//返回的是向量

	if (opt == false) {
		printf("CACHE_MANAGER_THREAD: can not get module %s_%s from cache\n", familyName.data(), name.data());
		ModuleInfo mod;
		mod.startTime = -1;
		mod.endTime = -1;
		return mod;
	}
	else {
		printf("CACHE_MANAGER_THREAD: get module %s_%s from cache\n",familyName.data(), name.data());
		vector<string>& mod_strs = *(vector<string>*)modP;
		vector<ModuleInfo> modV;
		if (mod_strs.size() == 0) {
			printf("CACHE_MANAGER_THREAD: can not get module %s_%s from cache\n", familyName.data(), name.data());
			ModuleInfo mod;
			mod.startTime = -1;
			mod.endTime = -1;
			return mod;
		}
		for (int i = 0;i < mod_strs.size();i++) {
			ModuleInfo mod;
			string s = mod_strs.at(i);
			mod.DeSerialize(s.data());
			modV.push_back(mod);
		}
		if(!modP)delete modP;
		delete[] key_;
		return modV.at(0);
	}
}


void CacheManager::insertModuleInfo(string familyName, string name, INT64 timestamp) {
	ModuleInfo mod;
	mod.startTime = timestamp;
	mod.endTime = -1;
	mod.SetPropertys();
	string s(mod.Serialize());
	char* objStr = new char[strlen(s.data()) + 1];
	strncpy(objStr, s.data(), s.length() + 1);
	string key = familyName + "_" + name;
	char* key_ = new char[strlen(key.data()) + 1];
	strncpy(key_, key.data(), key.length() + 1);
	const char* args[] = { "rpush", key_, objStr };

	int arg_nums = 3;
	void* res = NULL;
	this->cacheConnect.setCommndWithArgs(arg_nums, args, SET_STR, res);
	if (res != NULL)delete res;
	delete[] key_, objStr;


	return;

}

void CacheManager::updateModuleInfo(string familyName, string name, ModuleInfo mod_, INT64 endTime) {
	ModuleInfo mod;
	mod.SetPropertys();
	mod.startTime = mod_.startTime;
	mod.endTime = endTime;
	string s(mod.Serialize());
	char* objStr = new char[strlen(s.data()) + 1];
	strncpy(objStr, s.data(), s.length() + 1);

	string key = familyName + "_" + name;
	char* key_ = new char[strlen(key.data()) + 1];
	strncpy(key_, key.data(), key.length() + 1);
	const char* args[] = { "lset", key_, "-1", objStr };

	int arg_nums = 4;
	void* res = NULL;
	this->cacheConnect.setCommndWithArgs(arg_nums, args, SET_STR, res);
	if (res != NULL)delete res;
	delete[] key_, objStr;


	return;

}

void CacheManager::forceSetEndTime(bool flag) {
	string familyName;
	string name;
	vector<FamilyStruct> fst_vec = this->getFamilyInCache_pri(GET_LIST);
	for (int i = 0;i < fst_vec.size();i++) {
		FamilyStruct family_st = fst_vec.at(i);
		familyName = family_st.getName();
		for (int j = 0;j < family_st.nameList.size();j++) {
			name = family_st.nameList.at(j)->name;
			ModuleInfo mod = this->getLastModuleInfo(familyName, name);
			if (mod.startTime != -1 && mod.endTime == -1) {
				if (flag == true) {
					// Do error marking at the init time
					INT64 endTime = -2;
					this->updateModuleInfo(familyName, name, mod, endTime);
				}
				else {
					// Force set end time at abnormal closing
					auto time_now = chrono::system_clock::now();
					auto duration_in_ms = chrono::duration_cast<chrono::milliseconds>(time_now.time_since_epoch());
					INT64 endTime = (INT64)duration_in_ms.count();
					this->updateModuleInfo(familyName, name, mod, endTime);
				}
			}

		}
	}

}

