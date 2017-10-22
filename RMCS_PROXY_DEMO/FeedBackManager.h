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
	//���е�feedback�Ѿ�������������
	//��Ϊ�����Ķ����������߳���,���������������߳�֮��
	//��Ҫ����ɶ�����ʽ
	//1.��ҪfeedBackQueue
	//2.��Ҫ�ǵ�����
	//3.���д�����

private:
	queue_safe<GroupfeedbackCustomStruct>& group_feedback_queue;
	
public:
	 FeedBackManager(queue_safe<GroupfeedbackCustomStruct>& group_feedback_queue_);
	 //��������
	~FeedBackManager();
	GroupfeedbackCustomStruct toGroupFbCustomStruct(const GroupFeedback* group_fdb,string groupName);//ת������
	FeedbackCustomStruct toFeedBackCustomStruct(const Feedback& fdb); //��ÿ��ģ���fbת���ɿɶ�����
	void putToQueue(GroupfeedbackCustomStruct group_fdb_custom);//�Ž����У��������Ҫ��Щ������
	


};





#endif