#ifndef  COMMON_H
#define  COMMON_H

#define PUT_LIST "put_list"
#define GET_LIST "get_list"
#define GET_STR  "get_str"
#define ADD_VALUE_TO_SET  "add_value_to_set"
#define DELETE_VALUE_TO_SET "delete_value_to_set"
#define SET_STR "set_str"
#define DEL_KEY "del_key"
#define DEFAULT_SLEEP_TIME 100
#define DEFAULT_FD_FREQUENCY 1
#define LED_CMD 0
#define POSITION_CMD 1
#define TURQUE_CMD 2
#define VELOCITY_CMD 3

#include <iostream>
#include "JsonObjectBase.h"
#include <stdlib.h>
#include <stdio.h>
#include <mutex>
#include <string>

#include <vector>
#include <map>
#include <tacopie/tacopie>
#include <cpp_redis/cpp_redis>
#ifdef _WIN32
	#include <Winsock2.h>
#endif /* _WIN32 */
using namespace std;
class ModuleInfo :public CJsonObjectBase {
public:
	INT64 startTime;
	INT64 endTime;

	ModuleInfo::ModuleInfo() :startTime(-1), endTime(-1) {
		this->SetPropertys();
	}

	ModuleInfo::~ModuleInfo() {};

	virtual void SetPropertys() {
		this->clearProperty();
		this->SetProperty("startTime", CJsonObjectBase::asInt64, &startTime);
		this->SetProperty("endTime", CJsonObjectBase::asInt64, &endTime);
	}
};

class NameStruct :public CJsonObjectBase  {
public:
	string name;
	int connected;
	NameStruct(string name_, bool connect_) :name(name_), connected(connect_) {
		this->SetPropertys();
	
	}
	NameStruct() :name(""), connected(0) {
		this->SetPropertys();

	}
	~NameStruct() {}
	virtual void SetPropertys() {
		//
		this->clearProperty();
		this->SetProperty("name", CJsonObjectBase::asString, &(this->name));
		this->SetProperty("connected", CJsonObjectBase::asInt, &(this->connected));
	}
};
class FamilyStruct :public CJsonObjectBase  {
public:
	string name;
	vector<NameStruct*> nameList;

public:
	FamilyStruct():name(""),nameList() {
		this->SetPropertys();

	}
	FamilyStruct(string name) :nameList() {
		this->name = name;
		this->SetPropertys();
	}
	~FamilyStruct() {

	}
	void freeStruct(){
	
		for (int i = 0; i < nameList.size(); i++) {
			delete nameList.at(i);
		}
	
	
	}


	void setName(string name) { this->name = name; }
	string getName() { return name; }
	void addName(string name) {
		NameStruct* name_ = new NameStruct(name,false);
		nameList.push_back(name_);
		// Get all the names of members in a string, with ';' as spliter
	}
	void setNameList(vector<NameStruct*> namelist) {
		this->nameList = namelist;
	}
	vector<NameStruct*>   getNameList() { return nameList; }
	CJsonObjectBase* GenerateJsonObjForDeSerialize(const string& propertyName)
	{
		if ("nameList" == propertyName)
		{
			return (NameStruct*)(new NameStruct());
		}
		return NULL;
	}
	virtual void SetPropertys() {
		//
		this->clearProperty();
		this->SetProperty("name", CJsonObjectBase::asString, &(this->name));
		this->SetProperty("nameList", CJsonObjectBase::asVectorArray, &(this->nameList),CJsonObjectBase::asJsonObj);
	}
};


class GroupStruct :public CJsonObjectBase  {
public:
	string name;
	vector<FamilyStruct*> familyList;
	//初始表初始化

public:
	GroupStruct() :familyList(), name("") {
		this->SetPropertys();
	}

	GroupStruct(string name_) :familyList(), name(name_) {
		this->SetPropertys();
	
	}
	~GroupStruct() {
	}
	void freeStruct() {
		for (int i = 0; i < familyList.size(); i++) {
			familyList.at(i)->freeStruct();
			delete familyList.at(i);
		}	
	}
	void setName(string name) { this->name = name; }
	string getName() { return name; }
	void addFamily(FamilyStruct* family) {
		familyList.push_back(family);
	}
	vector<FamilyStruct*> getFamilyList() { return familyList; }
	virtual void SetPropertys() {
		//
		this->clearProperty();
		this->SetProperty("name", CJsonObjectBase::asString, &(name));
		this->SetProperty("familyList", CJsonObjectBase::asVectorArray, &(familyList),CJsonObjectBase::asJsonObj);
	}
	CJsonObjectBase* GenerateJsonObjForDeSerialize(const string& propertyName)
	{
		if ("familyList" == propertyName)
		{
			return (CJsonObjectBase*)(new FamilyStruct());
		}
		return NULL;
	}

};

class Led_field:public CJsonObjectBase   {
public:
	int led_R;
	int led_G;
	int led_B;
	Led_field(uint8_t R, uint8_t G, uint8_t B) :led_R(R), led_G(G), led_B(B) {
		this->SetPropertys();

	}
	Led_field() :led_R(0), led_G(0), led_B(0) {
		this->SetPropertys();

	}
	virtual void SetPropertys() {
		this->clearProperty();
		this->SetProperty("led_R", asInt, &led_R);
		this->SetProperty("led_G", asInt, &led_G);
		this->SetProperty("led_B", asInt, &led_B);
	}

	~Led_field() {

	}
};
class Actuator_field :public CJsonObjectBase {

public:
	double position;
	double velocity;
	double torque;
	double voltage;
	double motorCurrent;
	Actuator_field(double p, double v, double t, double vol, double motoCurrent_) :position(p), velocity(v), torque(t), voltage(vol), motorCurrent(motoCurrent_) {
		this->SetPropertys();
	}
	Actuator_field() :position(0), velocity(0), torque(0), voltage(0), motorCurrent(0) {
		this->SetPropertys();
	}
	~Actuator_field() {



	}
	virtual void SetPropertys() {
		this->clearProperty();
		this->SetProperty("position", asDouble, &position);
		this->SetProperty("velocity", asDouble, &velocity);
		this->SetProperty("torque", asDouble, &torque);
		this->SetProperty("voltage", asDouble, &voltage);
		this->SetProperty("motorCurrent", asDouble, &motorCurrent);
	}

};



//group里面module的重置格式
//反正只要我们需要的就好
class FeedbackCustomStruct:public CJsonObjectBase   {

public:

	Led_field led_field;
	Actuator_field actuator_field;
	FeedbackCustomStruct(Led_field l_f, Actuator_field a_f) :led_field(l_f.led_R, l_f.led_G, l_f.led_B), actuator_field(a_f.position, a_f.velocity, a_f.torque, a_f.voltage,a_f.motorCurrent) {
		this->SetPropertys();

	}
	FeedbackCustomStruct():led_field(0,0,0),actuator_field(0,0,0,0,0) {
		this->SetPropertys();
	}
	virtual void SetPropertys() {
		this->led_field.SetPropertys();
		this->actuator_field.SetPropertys();
		this->clearProperty();
		this->SetProperty("led_field", asJsonObj, &led_field);
		this->SetProperty("actuator_field", asJsonObj, &actuator_field);

	}
	~FeedbackCustomStruct() {


	}
};
//groupFeedBack的重置格式
class GroupfeedbackCustomStruct:public CJsonObjectBase   {



public:
	vector<double> positionsVec;
	vector<double> velocitysVec;
	vector<double> torqueVec;
	string groupName;
	INT64 timeStamp;
	vector<FeedbackCustomStruct*> moduleFeedBackVec;

	GroupfeedbackCustomStruct(vector<double> pVec,
	vector<double> vVec, vector<double> tVec, string groupName_) :
		positionsVec(pVec),
		velocitysVec(vVec),
		torqueVec(tVec),
		moduleFeedBackVec(),
		groupName(groupName_)
	{
		this->SetPropertys();

	}
	virtual void SetPropertys() {
		for (int i = 0; i < this->moduleFeedBackVec.size();i++) {
			moduleFeedBackVec.at(i)->SetPropertys();
		}
		this->clearProperty();
		this->SetProperty("positionsVec", CJsonObjectBase::asVectorArray, &positionsVec,CJsonObjectBase::asDouble);
		this->SetProperty("velocitysVec", CJsonObjectBase::asVectorArray, &velocitysVec, CJsonObjectBase::asDouble);
		this->SetProperty("torqueVec", CJsonObjectBase::asVectorArray, &torqueVec, CJsonObjectBase::asDouble);
		this->SetProperty("groupName", CJsonObjectBase::asString, &groupName);
		this->SetProperty("moduleFeedBackVec", CJsonObjectBase::asVectorArray, &moduleFeedBackVec,CJsonObjectBase::asJsonObj);
		this->SetProperty("timeStamp", CJsonObjectBase::asInt64, &timeStamp);

	}
	CJsonObjectBase* GenerateJsonObjForDeSerialize(const string& propertyName)
	{
		if ("moduleFeedBackVec" == propertyName)
		{
			return (CJsonObjectBase*)(new FeedbackCustomStruct());
		}
		return NULL;
	}
	~GroupfeedbackCustomStruct() {

	}
	void freeStruct() {
		for (int i = 0; i < moduleFeedBackVec.size(); i++) {
			delete moduleFeedBackVec.at(i);
		}	
	}

	bool putIntoModuleFeedBackVec(vector<FeedbackCustomStruct*>& fd_custom) {
		this->moduleFeedBackVec.clear();
		this->moduleFeedBackVec.assign(fd_custom.begin(), fd_custom.end());
		return true;
	}


};
class CommandStruct:public CJsonObjectBase {
public:
	Actuator_field actuator_field;
	Led_field led_field;
	CommandStruct(Actuator_field a, Led_field l) :actuator_field(a), led_field(l) {
		this->SetPropertys();

	}
	CommandStruct() :actuator_field(0,0,0,0,0), led_field(0,0,0) {
		this->SetPropertys();

	}
	~CommandStruct() {


	}
	virtual void SetPropertys() {
		this->clearProperty();
		this->SetProperty("led_field", asJsonObj, &led_field);
		this->SetProperty("actuator_field", asJsonObj, &actuator_field);

	}

};
class CommandGroupStruct :public CJsonObjectBase  {
public:

	string groupName;
	vector<string> familys;
	vector<string> names;
	vector<CommandStruct*> fd;
	int cmd;
	CommandGroupStruct() {
		this->SetPropertys();
	};
	CommandGroupStruct(vector<CommandStruct*> fd_,vector<string> fs_,vector<string> names_) :fd(fd_),familys(fs_),names(names_) {
		this->SetPropertys();
	}
	CommandGroupStruct(int cmd_,string groupName_,vector<CommandStruct*> fd_, vector<string> fs_, vector<string> names_)
		:fd(fd_), 
		familys(fs_), 
		names(names_),
		cmd(cmd_),
		groupName(groupName_)
		{
		this->SetPropertys();
	}
	~CommandGroupStruct() {
	}
	void freeStruct() {
		for (int i = 0; i < fd.size();i++) {
			delete fd.at(i);
		}
	}

	virtual void SetPropertys() {
		for (int i = 0; i < fd.size();i++) {
			this->fd.at(i)->SetPropertys();
		
		}
		this->clearProperty();
		this->SetProperty("groupName", asString, &groupName);
		this->SetProperty("familys", asVectorArray, &familys,asString);
		this->SetProperty("names", asVectorArray, &names,asString);
		this->SetProperty("fd", asVectorArray, &fd,asJsonObj);
		this->SetProperty("cmd", asInt, &cmd);
	}
	CJsonObjectBase* GenerateJsonObjForDeSerialize(const string& propertyName)
	{
		if ("fd" == propertyName)
		{
			CommandStruct* com = new CommandStruct();
			com->SetPropertys();
			return com;
		}
		return NULL;
	}
	bool putIntoModuleCommandVec(vector<CommandStruct*>& fd_custom) {
		this->fd.clear();
		this->fd.assign(fd_custom.begin(), fd_custom.end());
		return true;
	}



};



class RedisCofig {
public:
	string ip;
	int port;
	string password;
};
class DBconfig {
public:
	string user;
	string pwd;
	string ip;
	int port;
	string dbSchme;
	map<string, string> parmMap;
	DBconfig() :parmMap() {}
};
class CacheConnection {
	/*
	这个类管理底层对缓存的连接
	init连接缓存
	get/set 方法

	*/
	
private:
	string ip;
	int port;
	cpp_redis::future_client client;
	mutable std::mutex mut;
	
public:
	CacheConnection(string ip_,int port_):ip (ip_),port(port_){
	
	}
	CacheConnection() {
	
	}
	~CacheConnection() {
		
	}

	bool connect() {
		#ifdef _WIN32
		//! Windows netword DLL init
		WORD version = MAKEWORD(2, 2);
		WSADATA data;

		if (WSAStartup(version, &data) != 0) {
			std::cerr << "WSAStartup() failure" << std::endl;
			
			return false;
		}
		#endif /* _WIN32 */
		printf("connecting to redis [ip:%s,port:%d]\n", this->ip.data(), this->port);
		CacheConnection* this_ = this;
		client.connect(this->ip, this->port, [&this_](cpp_redis::redis_client&) {
			printf("failed connect to redis [ip:%s,port:%d]\n", this_->ip.data(), this_->port);
		
		});

		return client.is_connected();
	

	}//连接
	bool reconnect() {
		if (this->isConnected()) {
			return true;

		}
		else
		{
			CacheConnection* this_ = this;
			client.connect(this->ip, this->port, [&this_](cpp_redis::redis_client&) {
				printf("failed connect to redis [ip:%s,port:%d]\n", this_->ip.data(), this_->port);

			});
			return client.is_connected();
		}
	

	}//重新连接
	bool disconnect() {
		client.disconnect(true);
		return true;
	}//断开
	bool isConnected() {

		return this->client.is_connected();


	}//是否连接
	bool init() {
		return this->connect();

	}// 连接
	void setIpAndPort(string ip_, int port_) {
		this->ip = ip_;
		this->port = port_;
	}//重新设置
	bool isContainKey(char*  key) {
		void* intP=NULL;
		int arg_nums = 2;
		const char* args[] = { "exists",key };
		this->setCommndWithArgs(arg_nums, args, GET_STR, intP);
		return (intP==NULL) || (*(int*)intP == 0) ? false : true;
	}//是否包含Key
	bool deleteKey(char*  key) {
		void* intP;
		int arg_nums = 2;
		const char* args[] = { "del",key };
		this->setCommndWithArgs(arg_nums, args, DEL_KEY, intP);
		delete[] args;
		return (*(int*)intP == 0) ? false : true;


	}//删除键
	bool setCommndWithArgs(int agr_nums, const char** args, string des, void* &res) {
		if (this->isConnected() == false) return false;
		std::lock_guard<std::mutex> lk(mut);
		vector<string> argVe;
		for (int i = 0; i < agr_nums; i++) {
			argVe.push_back(args[i]);
		}
		std::future<cpp_redis::reply> rpF = client.send(argVe);//组织命令
		client.sync_commit(std::chrono::milliseconds(3000));//3秒钟延迟发送
		cpp_redis::reply rp = rpF.get();//等待返回


		cpp_redis::reply::type t = rp.get_type();
		if (t == cpp_redis::reply::type::array) {
			vector<string>* vec = new vector<string>();
			vector<cpp_redis::reply> rVec = rp.as_array();
			for (int i = 0; i < rVec.size(); i++) {
				vec->push_back(rVec.at(i).as_string());
			}
			res = (void*)vec;
			return true;
		}
		else if (t == cpp_redis::reply::type::integer) {
			//条数,比如插入删除什么的
			int* a = new int;
			*a = rp.as_integer();
			res = (void*)a;
			return true;
		}
		else if (t == cpp_redis::reply::type::simple_string) {
			//返回的字符串
			string* a = new std::string(rp.as_string());
			res = (void*)a;
			return true;
		}
		else if (t == cpp_redis::reply::type::bulk_string) {
			//返回的字符串
			string* a = new std::string(rp.as_string());
			res = (void*)a;
			return true;
		}
		else if (t == cpp_redis::reply::type::null) {
			//空值对象
			//string s = rp.error();
			string outString("REDIS__COMMAND_ERROR: error null ,the redis command is [ ");
			for (int i = 0; i < agr_nums; i++) {
				outString.append(args[i]);
				outString.append(" ");
			}
			outString.append(" ]\n");
			printf(outString.data());
			return false;
		}
		else if (t == cpp_redis::reply::type::error) {
			//错误
			string s = rp.error();
			string outString("REDIS__COMMAND_ERROR: error null ,the redis command is [ ");
			for (int i = 0; i < agr_nums; i++) {
				outString.append(args[i]);
				outString.append(" ");
			}
			outString.append(" ]\n");
			printf(outString.data());
			return false;

		}
		else {
			string outString("REDIS__COMMAND_ERROR:unknowen error ,the redis command is [ ");
			for (int i = 0; i < agr_nums; i++) {
				outString.append(args[i]);
				outString.append(" ");
			}
			outString.append(" ]\n");
			printf(outString.data());
			return false;


		}


	}
	bool authPwd(string password) {
		if (this->isConnected() == false) return false;
		std::lock_guard<std::mutex> lk(mut);
		std::future<cpp_redis::reply> rpF = client.auth(password); 
		client.sync_commit(std::chrono::milliseconds(3000));//3秒钟延迟发送
		cpp_redis::reply rp = rpF.get();//等待返回
		try
		{
			if (rp.is_error() == false) {
				//没出错
				printf("redis password is right [password:%s]\n", password.data());
				return true;
			}
			else {
				printf("redis password is wrong [password:%s]\n", password.data());
				return false;
			}
		}
		catch (const std::exception&)
		{
			printf("redis auth error! [password:%s]\n", password.data());
			return false;
		}


	}
	
	//发送命令
	
	
};
class DBconnection {
	//数据库连接类



};
class ServerConfig {
public:
	string ip;
	int port;
	ServerConfig(string ip_,int port_):ip(ip_),port(port_) {
		
	}
	ServerConfig() :ip(""), port(0) {

	}

};

#endif