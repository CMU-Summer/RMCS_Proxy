#include "FeedBackManager.h"
#include "include/json/json.h"

FeedBackManager::FeedBackManager(queue_safe<GroupfeedbackCustomStruct>& group_feedback_queue_):
	group_feedback_queue(group_feedback_queue_){



}
//往进来放
FeedBackManager::~FeedBackManager(){
	



}
GroupfeedbackCustomStruct FeedBackManager::toGroupFbCustomStruct(const GroupFeedback* group_fdb,string groupName){
	printf( "change gfd to custom_gfd\n");
	
   // positionVec,velVec,torqueVec 无意义
   Eigen::VectorXd vxd= group_fdb->getPosition();//列向量，1列多行
   Eigen::VectorXd vxd1= group_fdb->getEffort();//列向量，1列多行
   Eigen::VectorXd vxd2= group_fdb->getVelocity();//列向量，1列多行
 
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
   //获得group的位置速度扭矩


   vector<FeedbackCustomStruct*> moduleVec;
   for(int j=0;j<group_fdb->size();j++){
	   //获得每一个的
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



}//转换函数



FeedbackCustomStruct FeedBackManager::toFeedBackCustomStruct(const Feedback& fdb){
	printf("change fd to custom_fd\n");
	Actuator_field a(fdb.actuator().position().get(),fdb.actuator().velocity().get(),fdb.actuator().effort().get(),fdb.voltage().get(),fdb.actuator().motorCurrent().get());
	Led_field l(fdb.led().getColor().getRed(),fdb.led().getColor().getGreen(),fdb.led().getColor().getBlue());
	FeedbackCustomStruct fdb_custom(l,a);
	return fdb_custom;
} //吧每个模块的fb转换成可读化的
void FeedBackManager::putToQueue(GroupfeedbackCustomStruct group_fdb_custom){
	this->group_feedback_queue.push(group_fdb_custom);


}//放进队列，这里面就要锁些东西了



