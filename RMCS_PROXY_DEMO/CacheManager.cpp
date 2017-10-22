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
				//ȥ��֤����
				bool opt=this->cacheConnect.authPwd(cogVec.at(0).password);
				if (opt) {
					printf("CACHE_MANAGER_THREAD: redis auth successfully \n");
					this->isConnect = true;
				}
				else {
					printf("CACHE_MANAGER_THREAD: redis auth failed\n");
					this->isConnect = false;//��֤ûͨ������û��
				}
			
			}



			printf("CACHE_MANAGER_THREAD: redis connect status:%d\n", this->isConnect);
		}
		catch (exception* e)
		{
			printf("CACHE_MANAGER_THREAD: redis connect error.\n");
		}
	}

	this->start();//���߳�


}//���ӣ����̵߳ȵȲ���

vector<GroupStruct> CacheManager::getGroupInCache(){
	//����߳�����
	//cout << "get user groups from cache" << endl;
	printf("LOOKUPMANAGER_THREAD: get user groups from cache\n");
	return this->getGroupInCache_pri(GET_LIST);

}//��ȡcache�е�list,��Ҫ��

vector<FamilyStruct> CacheManager::getFamilyInCache() {
	printf("LOOKUPMANAGER_THREAD: get families from cache\n");
	return this->getFamilyInCache_pri(GET_LIST);
	
}

bool CacheManager::updateCacheFamilyAndItsNames(map<string,vector<string>> familyMap){
	//�Ѿ��ڶ��к����д�������

	printf("LOOKUPMANAGER_THREAD: put family and its names into queue\n");
	if(this->cacheConnect.isConnected() == false)return false;
	else {
		this->family_name_queue.push(familyMap);
		return true;
	}

}
//�����ڲ��������
//����������ϵģ���ô���ص���true
//�������ͨ���ڱ���̵߳�

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
//�������
//��
bool CacheManager::isConnected(){return this->cacheConnect.isConnected();}//�Ƿ����������ϵ�
void CacheManager::customfamilyMap(){
	int index = 0;
	int size = family_name_queue.size();
	while (index<size)
	{
		shared_ptr<map<string, vector<string>>> mapPtr = this->family_name_queue.try_pop();
		if (!mapPtr)return;//û��ȡ��

		printf("CACHE_MANAGER_THREAD: get family and names from queue\n");
		map<string, vector<string>>::iterator it;
		for (it = mapPtr->begin(); it != mapPtr->end(); it++) {
			//�Ž�family�ļ�������ȥ
			//����������family��set ��group �� set 
			this->flushCacheAndItNameList(it->first, it->second);
		}
		index++;
	}

}//���Ķ��������familyMap����
void CacheManager::customGroupStateMap(){
	//
	int conut = 0;
	int size = group_struct_queue.size();
	while (conut<size) {
		shared_ptr<vector<GroupStruct>> mapPtr = this->group_struct_queue.try_pop();
		if (!mapPtr)return;//û��ȡ����Ҫ���ĵ�groupVec


		printf("CACHE_MANAGER_THREAD: get group status from queue\n");
		vector<GroupStruct>::iterator it;
		for (it = mapPtr->begin(); it != mapPtr->end(); it++) {

			//��֮ǰҪ���ж��£����group���ڲ��ڻ��������ˣ���Ϊ�п��ܱ�ɾ��
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

} // ���Ķ��������groupStructVec����
void CacheManager::setIpAndPort(string ip_,string port_){
	this->cacheConnect.setIpAndPort(ip_, std::atoi(port_.data()));
}//��������
void CacheManager::run(){
	while (true)
	{
		if (stop_flag) { // if stop flag received
			this->forceSetEndTime(false);
			return;
		}
		//cout<<"lookupManager thread work"<<endl;
		printf("CACHE_MANAGER_THREAD: cache manager thread work\n");
		customfamilyMap();//�����¶���
		customGroupStateMap();
		customGroupFeedBack();
		if(this->sleep_time>0){
			this_thread::sleep_for(std::chrono::milliseconds(this->sleep_time));
		}
		printf("CACHE_MANAGER_THREAD: cache manager is ready for next\n");
	}
}//�������к���

bool CacheManager::reconnect(){
	if(this->cacheConnect.isConnected()==false){
		return this->isConnect=this->cacheConnect.reconnect();
	}else
	{
		return this->isConnect = true;
		
	}
}//��������

vector<FamilyStruct> CacheManager::getFamilyInCache_pri(string des) {
	void* familyP = NULL;
	int arg_nums = 2;
	const char* args[] = { "smembers","family", };
	bool opt = this->cacheConnect.setCommndWithArgs(arg_nums, args, des, familyP);//���ص�������

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
		f.setName(f_strs.at(i).data()); //����family_name
		//ȡstr
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
	return familyV; //����

}


vector<GroupStruct> CacheManager::getGroupInCache_pri(string des){
		//��ȡ group��list,��ȡstr
		void* groupP=NULL;
		int arg_nums=2;
		const char* args[]={"smembers","group",};
		bool opt = this->cacheConnect.setCommndWithArgs(arg_nums,args,des,groupP);//���ص�������
			
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
			//ȡstr
			void* str=NULL;
			char* group_key_ = new char[strlen(g_strs.at(i).data())+1];
			int arg_nums_1=2;
			const char* args_1[]={"get",strncpy(group_key_,g_strs.at(i).data(),g_strs.at(i).length()+1)};
			this->cacheConnect.setCommndWithArgs(arg_nums_1,args_1,GET_STR,str);//str�Ѿ�����һ��str��
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
		return groupV; //����

	


}//ȡ����
//���û����࣬�Ž�ȥ
bool CacheManager::flushCacheAndItNameList(const string& key ,vector<string>& list){
	//����set��Ҫadd 
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
	const char* args_[]={"sadd","family",strncpy(key__,key.data(),key.length()+1)};//�ŵ�family set����ȥ
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
	//�ж���key ��set str
	 char* key_=new char[strlen( gst.getName().data())+1];
	 gst.SetPropertys();//����һ��
	 string s= gst.Serialize();

	char* objStr=new char[strlen(s.data())+1];
	const char* args_[]={"set",strncpy(key_,gst.getName().data(),gst.getName().length()+1),strncpy(objStr,s.data(),s.length()+1)};//�ŵ�family set����ȥ
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
	//�ж���key ��set str
	string gfd_name_key=gfd.groupName+"_gfd";//group��fdĬ�ϼӸ���׺
	char* key_=new char[strlen(gfd_name_key.data())+1];
	gfd.SetPropertys();
	string s(gfd.Serialize());
	char* objStr=new char[strlen(s.data())+1];
	const char* args_[]={"rpush",strncpy(key_,gfd_name_key.data(),gfd_name_key.length()+1),strncpy(objStr,s.data(),s.length()+1)};//�ŵ�groupfdlist set����ȥ
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



}//ˢ��Զ�˻��������groupFeedback
bool CacheManager::updateGroupFeedBack(GroupfeedbackCustomStruct gfd){
	printf("FDBKCUSTOMER__THREAD: put group feed back into queue\n");
	//cout<<"put group feed back into queue"<<endl;
	if(this->cacheConnect.isConnected() == false)return false;
	else {
		this->group_feedback_queue.push(gfd);
		return true;
	}

}//���»���feedbacklist,ֻ������
void CacheManager::customGroupFeedBack(){
	int count=0;
	int size = group_feedback_queue.size();
	while (count<size)
	{
		shared_ptr<GroupfeedbackCustomStruct> mapPtr = this->group_feedback_queue.try_pop();
		if (!mapPtr)return;//û��ȡ����Ҫ���ĵ�groupVec
		printf("CACHE_MANAGER_THREAD: get one feedback from queue\n");
		this->flushCacheGroupFeedBackList(*mapPtr);
		mapPtr = this->group_feedback_queue.try_pop();
		count++;
	}

}
string CacheManager::getGroupCommandJsonStrFromCache() {
	//���������ȡ�ַ�������
	void* str;
	int argNum = 2;
	const char* a[] = {"lpop","command_c"};//�����command_c����
	if (this->isConnect == false)return "";
	if (this->cacheConnect.setCommndWithArgs(2, a, GET_STR, str)) {
		
		string s((*(string*)str));
		delete str;
		return s;
	}
	else {
		//�ɹ��� str����һ��string
		//��ȡʧ����
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
	if (!res)delete res; //�ͷſռ�
	delete[] key_; //ɾ����
}
 //���Ķ��������groupFeedBack

ModuleInfo CacheManager::getLastModuleInfo(string familyName, string name) {
	void* modP = NULL;
	int arg_nums = 4;

	string key = familyName + "_" + name;
	char* key_ = new char[strlen(key.data()) + 1];
	strncpy(key_, key.data(), key.length() + 1);
	const char* args[] = { "lrange", key_,"-1", "-1" };

	bool opt = this->cacheConnect.setCommndWithArgs(arg_nums, args, GET_LIST, modP);//���ص�������

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

