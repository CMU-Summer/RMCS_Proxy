#ifndef COMMANDCUSTOMER_H
#define COMMANDCUSTOMER_H
#include "CThread.h"
#include <string>
#include "queue_safe.h"
#include "common.h"
#include "ConfigManager.h"
#include "LookUpManager.h"
#include "lookup.hpp"
using namespace  std;
class CommandCustomer:public CThread
{
	//����һ���߳��࣬
	//��ͣ������commandQueue�����е�����
	//commandQueue�����У�ÿһ��Ԫ�أ���ָ����Ҫ���������group�������е�module
	//�Լ�����ĸ�ʽ
	//Ԫ�ص�������Ҫ�����ﶨ��
	//feedback�ᱻfeedbackcustomer����

public:
	CommandCustomer(
		queue_safe<CommandGroupStruct>& command_struct_queue_,
		ConfigManager& cfg,
		map<string, unique_ptr<hebi::Group>>& cacheGroupMap_,//��Ҫ���µ�map,�ӻ�������ȡ�����keyû�о�ȡһ��addFh,�оͲ���
		map<string, unique_ptr<hebi::Group>>& fixedGroupMap_,
		int sleepTime_=DEFAULT_SLEEP_TIME
	):command_struct_queue(command_struct_queue_),
	  cfgManager(cfg), 
	  sleeptime(sleepTime_),
	  cacheGroupMap(cacheGroupMap_),
	  fixedGroupMap(fixedGroupMap_)
	  {


	}
	~CommandCustomer() {
	
	
	
	
	}
	CommandGroupStruct* getFakeLedCommand(vector<string> fs_, vector<string> ns_, string groupName, int size);
	void run() override; //��дrun
	void init(){
		this->start();


	}//


	hebi::GroupCommand getGroupCommandStrut(CommandStruct ct){




	}
	hebi::Command getCommandStrut(CommandStruct ct){




	}
	unique_ptr<hebi::Group>& getGroupPtr(string groupname) {
		//���Ѵ��ڵ�group�����ȡ
		try
		{
			if (this->cacheGroupMap.count(groupname)) {
				return this->cacheGroupMap[groupname];

			}
			else if (this->fixedGroupMap.count(groupname)) {
				return this->fixedGroupMap[groupname];

			}
			else {
				unique_ptr<Group> gr;
				return gr;//��ָ��
			}
		}
		catch (const std::exception&)
		{
			printf("COMMAND_____CUSTOMER: cache map error\n");
			unique_ptr<Group> gr;
			return gr;//��ָ��
		}
		
	
	}


	bool customCommand(){
		
		
		//shared_ptr<CommandGroupStruct> mapPtr  = this->command_struct_queue.wait_and_pop();

		CommandGroupStruct* mapPtr = getFakeLedCommand({"SEA-Snake","Spare"}, {"SA011","SA035"},"testGroup",2);
		
		if(mapPtr==nullptr )return false;//û��ȡ����Ҫ���ĵ�gfd
		


		unique_ptr<hebi::Group>& g= this->getGroupPtr(mapPtr->groupName);//�ҵõ�,�ͷ�������,����Ϊ��
		//printf("COMMAND_CUSTOMER : g is not null:%d\n",g!=NULL);
		if (g._Mypair._Get_second() == NULL ){
			printf("COMMAND_____CUSTOMER: group is  null\n");
			return false;
		}
		printf("COMMAND_____CUSTOMER: send led command\n");
		//�������group��command
		hebi::GroupCommand command(g->size());
		switch (mapPtr->cmd)
		{
		case LED_CMD: {
			for (int i = 0; i < g->size();i++) {
				if (mapPtr->fd.size() <= i)break;
				const hebi::Color color_(mapPtr->fd.at(i)->led_field.led_R, mapPtr->fd.at(i)->led_field.led_G, mapPtr->fd.at(i)->led_field.led_B);
				//command[i].led().setOverrideColor(color_);
			
			}
			break;
		
		}
		case POSITION_CMD: {
			for (int i = 0; i < g->size(); i++) {
				if (mapPtr->fd.size() <= i)break;
				command[i].actuator().position().set(mapPtr->fd.at(i)->actuator_field.position);

			}
			break;
		
		}
		case TURQUE_CMD:{
			for (int i = 0; i < g->size(); i++) {
				if (mapPtr->fd.size() <= i)break;
				command[i].actuator().effort().set(mapPtr->fd.at(i)->actuator_field.torque);

			}
			break;
		
		}
		case VELOCITY_CMD: {
			for (int i = 0; i < g->size(); i++) {
				if (mapPtr->fd.size() <= i)break;
				command[i].actuator().velocity().set(mapPtr->fd.at(i)->actuator_field.velocity);

			}
			break;

		}
		default:
			//do nothing
			break;
		}

		int timeout=100;
		if (g->sendCommandWithAcknowledgement(command, timeout))
		{
			
			printf("COMMAND_____CUSTOMER: already sent!\n");
		}
		else
		{
		
			printf("COMMAND_____CUSTOMER: send time out\n");
		}
		mapPtr->freeStruct();

		return true;
	}
private:
	queue_safe<CommandGroupStruct>& command_struct_queue;
	ConfigManager& cfgManager;

	int sleeptime;
	map<string, unique_ptr<hebi::Group>>& cacheGroupMap;//��Ҫ���µ�map,�ӻ�������ȡ�����keyû�о�ȡһ��addFh,�оͲ���
	map<string, unique_ptr<hebi::Group>>& fixedGroupMap;
};

void CommandCustomer::run(){
	while (true)
	{
		printf("COMMAND_____CUSTOMER: command customer working!!!!!\n");
		this->customCommand();
		if(this->sleeptime>0){
			this_thread::sleep_for(std::chrono::milliseconds(this->sleeptime));
		}
		printf("COMMAND_____CUSTOMER: command customer is ready for next!!!!!\n");
	}

}
CommandGroupStruct* CommandCustomer::getFakeLedCommand(vector<string> fs_,vector<string> ns_,string groupName,int size) {
	Led_field l(181, 181, 181);
	Actuator_field a(0,0,0,0,0);
	vector<CommandStruct*> v;
	for (int i = 0; i < size;i++) {
		CommandStruct* c=new CommandStruct(a, l);
		v.push_back(c);
	}

	CommandGroupStruct* cg = new CommandGroupStruct(v,fs_,ns_);
	cg->groupName.assign(groupName);
	cg->cmd = LED_CMD;
	return cg;

}

#endif // !
