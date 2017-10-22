#include "FeedBackManager.h"
#include "include/json/json.h"

FeedBackManager::FeedBackManager(queue_safe<GroupfeedbackCustomStruct>& group_feedback_queue_):
	group_feedback_queue(group_feedback_queue_){



}
//��������
FeedBackManager::~FeedBackManager(){
	



}
GroupfeedbackCustomStruct FeedBackManager::toGroupFbCustomStruct(const GroupFeedback* group_fdb,string groupName){
	printf( "change gfd to custom_gfd\n");
	
   // positionVec,velVec,torqueVec ������
   Eigen::VectorXd vxd= group_fdb->getPosition();//��������1�ж���
   Eigen::VectorXd vxd1= group_fdb->getEffort();//��������1�ж���
   Eigen::VectorXd vxd2= group_fdb->getVelocity();//��������1�ж���
 
   vector<double> positionVec,torqueVec,velVec;
   
   int i= vxd.rows();
   for(int j=0;j<i;j++){
	   positionVec.push_back(static_cast<double>( vxd[j]));
   }
   i= vxd1.rows();
   for(int j=0;j<i;j++){
	   torqueVec.push_back(static_cast<double>( vxd1[j]));
   }
   i= vxd2.rows();
   for(int j=0;j<i;j++){
	   velVec.push_back(static_cast<double>( vxd2[j]));
   }
   //���group��λ���ٶ�Ť��


   vector<FeedbackCustomStruct*> moduleVec;
   for(int j=0;j<group_fdb->size();j++){
	   //���ÿһ����
	   const Feedback& fdb_temp=(*group_fdb)[j];
	   FeedbackCustomStruct* fd_struct = new FeedbackCustomStruct(toFeedBackCustomStruct(fdb_temp));
	   moduleVec.push_back(fd_struct);
   }
   GroupfeedbackCustomStruct g(positionVec,velVec,torqueVec,groupName);
   g.putIntoModuleFeedBackVec(moduleVec);
   auto time_now = chrono::system_clock::now();
   auto duration_in_ms = chrono::duration_cast<chrono::milliseconds>(time_now.time_since_epoch());
   g.timeStamp = (INT64)duration_in_ms.count();

   return g;



}//ת������



FeedbackCustomStruct FeedBackManager::toFeedBackCustomStruct(const Feedback& fdb){
	printf("change fd to custom_fd\n");
	Actuator_field a(fdb.actuator().position().get(),fdb.actuator().velocity().get(),fdb.actuator().effort().get(),fdb.voltage().get(),fdb.actuator().motorCurrent().get());
	Led_field l(fdb.led().getColor().getRed(),fdb.led().getColor().getGreen(),fdb.led().getColor().getBlue());
	FeedbackCustomStruct fdb_custom(l,a);
	return fdb_custom;
} //��ÿ��ģ���fbת���ɿɶ�����
void FeedBackManager::putToQueue(GroupfeedbackCustomStruct group_fdb_custom){
	this->group_feedback_queue.push(group_fdb_custom);


}//�Ž����У��������Ҫ��Щ������



