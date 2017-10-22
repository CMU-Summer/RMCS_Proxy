#include "FeedBackCustomer.h"


void FeedBackCustomer::run(){
	while (true)
	{
		if (stop_flag) // if stop signal received
			return;
		customGfd();
		
		printf("FDBKCUSTOMER__THREAD: feedback Customer is ready for next execute\n");
	}


}

void FeedBackCustomer::init(){
	stop_flag = false;
	this->start();//跑起来
} //初始化


bool FeedBackCustomer::customGfd(){
	printf("FDBKCUSTOMER__THREAD: feedback customer is reay for next feedback !!!!\n");
	shared_ptr<GroupfeedbackCustomStruct> mapPtr  = this->gfd_quee.wait_and_pop();
	if(!mapPtr)return false;//没有取到需要消耗的gfd
	printGroupFeedBack(*mapPtr);//Feedback data display
	bool opt=this->insertCache(*mapPtr);
	bool opt1=this->insertDb(LOCAL,*mapPtr);
	bool opt2=this->insertDb(WEB,*mapPtr);
	
	return opt == opt2 == opt1 == true;
}

bool FeedBackCustomer:: insertCache(GroupfeedbackCustomStruct& gfd){
	printf("FDBKCUSTOMER__THREAD: put into cache!!!\n");
	return this->cacheManager.updateGroupFeedBack(gfd);
	

}//放进缓存里面
bool FeedBackCustomer:: insertDb(int type,GroupfeedbackCustomStruct& gfd){
	//cout<<"put into db!!!"<<endl;
	printf("FDBKCUSTOMER__THREAD: put fd into local db or web db!!!\n");
	return true;


}//放进数据库里面