#include "InitManager.h"
#include <random>
using namespace std::chrono;
int main() {
#ifdef _WIN32
	//! Windows netword DLL init
	WORD version = MAKEWORD(2, 2);
	WSADATA data;

	if (WSAStartup(version, &data) != 0) {
		std::cerr << "WSAStartup() failure" << std::endl;
		return -1;
	}
#endif /* _WIN32 */
	return InitManager::main();

//-----------------test code for put feedbcak to cache---------------
// 	default_random_engine e;
// 	e.seed(10); //种子
// 	uniform_int_distribution<unsigned> u_led(0, 253); //灯随机数
// 	uniform_real_distribution<double> u_current(0, 0.1); // 电流随机数
// 	uniform_real_distribution<double> u_position(-0.5, 0.5); // 位置随机数
// 	uniform_real_distribution<double> u_velocity(-0.3, 0.3); // 速度随机数
// 	uniform_real_distribution<double> u_torque(-0.3, 1.3); // 速度随机数
// 	ConfigManager cManager("resource/config.xml");
// 	CacheManager c(cManager,300);
// 	c.initCacheManager();
// 	int defalut_sleeptime=500;
// 	for (int k = 0; k < 1500;k++) {
// 		GroupfeedbackCustomStruct gfd({ 0,0 }, { 0,0 }, { 0,0 }, "group1_g");
// 		vector<FeedbackCustomStruct*> fdVec;
// 		for (int i = 0; i < 2; i++) {
// 		
// 			FeedbackCustomStruct* fdPtr = new FeedbackCustomStruct(Led_field(u_led(e), u_led(e), u_led(e)), Actuator_field(u_position(e),u_velocity(e) ,u_torque(e),u_velocity(e) ,u_current(e)));
// 			fdVec.push_back(fdPtr);
// 
// 		}
// 		gfd.putIntoModuleFeedBackVec(fdVec);
// 		auto time_now = chrono::system_clock::now();
// 		auto duration_in_ms = chrono::duration_cast<chrono::milliseconds>(time_now.time_since_epoch()); 
// 		gfd.timeStamp = (INT64)duration_in_ms.count();
// 		bool opt=c.updateGroupFeedBack(gfd);
// 		if (!opt) {
// 			cout << "put to queue false" << endl;
// 		}
// 		this_thread::sleep_for(std::chrono::milliseconds(defalut_sleeptime));
// 	}
//-----------------test code for put feedbcak to cache---------------



}

//-------------------test code for Serialize---------------------
// 	gfd.SetPropertys();
// 	string s(gfd.Serialize());
// 	vector<CommandStruct*> cmdVec;
// 	for (int i = 0; i < 2;i++) {
// 		CommandStruct* cmdPtr = new CommandStruct(Actuator_field(0,0,0,0,0),Led_field(0,0,0));
// 		cmdVec.push_back(cmdPtr);
// 	
// 	}
// 	CommandGroupStruct g_cmd(1, "testGroup", cmdVec, { "f1","f2" }, {"n1","n2"});
// 	g_cmd.SetPropertys();
// 	string s1(gfd.Serialize());
// 	printf("%s\n%s\n",s.data(),s1.data());
//-------------------test code for Serialize---------------------




// #include "winsock2.h"  
// #include <iostream>  
// #pragma comment(lib, "ws2_32.lib")  
// 
// using namespace std;
// BOOL RecvLine(SOCKET s, char* buf); //读取一行数据  
// 
// int main(int argc, char* argv[])
// {
// 	const int BUF_SIZE = 64;
// 
// 	WSADATA wsd; //WSADATA变量  
// 	SOCKET sHost; //服务器套接字  
// 	SOCKADDR_IN servAddr; //服务器地址  
// 	char buf[BUF_SIZE]; //接收数据缓冲区  
// 	char bufRecv[BUF_SIZE];
// 	int retVal; //返回值  
// 				//初始化套结字动态库  
// 	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
// 	{
// 		cout << "WSAStartup failed!" << endl;
// 		return -1;
// 	}
// 	//创建套接字  
// 	sHost = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
// 	if (INVALID_SOCKET == sHost)
// 	{
// 		cout << "socket failed!" << endl;
// 		WSACleanup();//释放套接字资源  
// 		return  -1;
// 	}
// 
// 	//设置服务器地址  
// 	servAddr.sin_family = AF_INET;
// 	servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
// 	servAddr.sin_port = htons((short)9293);
// 	int nServAddlen = sizeof(servAddr);
// 
// 	//连接服务器  
// 	retVal = connect(sHost, (LPSOCKADDR)&servAddr, sizeof(servAddr));
// 	if (SOCKET_ERROR == retVal)
// 	{
// 		cout << "connect failed!" << endl;
// 		closesocket(sHost); //关闭套接字  
// 		WSACleanup(); //释放套接字资源  
// 		return -1;
// 	}
// 	while (true)
// 	{
// 		//向服务器发送数据  
// 		ZeroMemory(buf, BUF_SIZE);
// 		cout << " 向服务器发送数据:  ";
// 		cin >> buf;
// 		retVal = send(sHost, buf, strlen(buf), 0);
// 		if (SOCKET_ERROR == retVal)
// 		{
// 			cout << "send failed!" << endl;
// 			closesocket(sHost); //关闭套接字  
// 			WSACleanup(); //释放套接字资源  
// 			return -1;
// 		}
// 		//RecvLine(sHost, bufRecv);  
// 		ZeroMemory(bufRecv, BUF_SIZE);
// 		recv(sHost, bufRecv, BUF_SIZE, 0); // 接收服务器端的数据， 只接收5个字符  
// 		cout << endl << "从服务器接收数据：" << bufRecv;
// 		cout << "\n";
// 	}
// 	//退出  
// 	closesocket(sHost); //关闭套接字  
// 	WSACleanup(); //释放套接字资源  
// 	int i = 0;
// 	cin >> i;
// 	return 0;
// }