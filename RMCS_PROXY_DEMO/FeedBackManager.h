#ifndef	FEEDBACKMANAGER_H
#define FEEDBACKMANAGER_H
#include <feedback.hpp>
#include <group_feedback.hpp>


#include "queue_safe.h"
#include <vector>
//#include "JsonObjectBase.h"
#include "common.h"
using namespace std;
using namespace hebi;

class FeedBackManager{
	//所有的feedback已经被管理起来了
	//认为这个类的对象运行在线程中,即允许在生产者线程之中
	//需要定义可读化形式
	//1.需要feedBackQueue
	//2.需要记得上锁
	//3.队列传进来

private:
	queue_safe<GroupfeedbackCustomStruct>& group_feedback_queue;
	
public:
	 FeedBackManager(queue_safe<GroupfeedbackCustomStruct>& group_feedback_queue_);
	 //往进来放
	~FeedBackManager();
	GroupfeedbackCustomStruct toGroupFbCustomStruct(const GroupFeedback* group_fdb,string groupName);//转换函数
	FeedbackCustomStruct toFeedBackCustomStruct(const Feedback& fdb); //吧每个模块的fb转换成可读化的
	void putToQueue(GroupfeedbackCustomStruct group_fdb_custom);//放进队列，这里面就要锁些东西了
	


};





#endif