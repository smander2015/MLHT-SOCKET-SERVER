#include <stdio.h>
#include <stdlib.h>


#define DriverInstallPath "C:\\WINDOWS\\system32\\drivers\\cccc.sys"


#define Install_Run 1
#define Stop_Unload 2


void installDvr();		//安装驱动
void startDvr();		//启动驱动
void stopDvr();			//停止驱动
void unloadDvr();		//卸载驱动


void setdriver(int op)
{
	if(op==Install_Run)
	{
		installDvr();	//安装驱动
		startDvr();		//启动驱动 
	}
	if(op==Stop_Unload)
	{
		stopDvr();		//停止驱动
		unloadDvr();	//卸载驱动
	}

}

void installDvr()		//安装
{
	SC_HANDLE schSCManager;
	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (schSCManager)
	{ 
		SC_HANDLE schService = CreateService( schSCManager,

			"babaHelloDDKk",

			"babaMyHelloDDKk",

			SERVICE_ALL_ACCESS,

			SERVICE_KERNEL_DRIVER, //创建的服务类型1为驱动服务

			SERVICE_DEMAND_START, //用于当有进程调用StartService 函数时由服务控制管理器(SCM)启动的服务。查询Starting Services on Demand以获取更多信息。

			SERVICE_ERROR_IGNORE,

			DriverInstallPath,//驱动文件存放路径

			NULL,

			NULL,

			NULL,

			NULL,

			NULL);

		if(schService)
		{
			printf("安装服务成功\n");
		}
		else
		{
			printf("安装服务失败\n");
		}
		CloseServiceHandle(schService); //创建完记得释放句柄
		CloseServiceHandle(schSCManager);
	}

}


void startDvr()//启动
{
	SC_HANDLE schSCManager;
	SC_HANDLE hs;
	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(schSCManager)
	{
		hs=OpenService(schSCManager, "babaHelloDDKk", SERVICE_ALL_ACCESS); //打开服务
		if (hs)
		{
			StartService(hs,0,0);
			printf("启动服务成功\n");

			CloseServiceHandle(hs);
		}
		CloseServiceHandle(schSCManager);
	}
}


void stopDvr()//停止
{
	SC_HANDLE schSCManager;
	SC_HANDLE hs;
	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(schSCManager)
	{
		hs=OpenService(schSCManager, "babaHelloDDKk", SERVICE_ALL_ACCESS); //打开服务
		if (hs)
		{
			SERVICE_STATUS status;
			int num=0;

			QueryServiceStatus(hs, &status);
			if (status.dwCurrentState != SERVICE_STOPPED && status.dwCurrentState != SERVICE_STOP_PENDING)
			{
				ControlService(hs,SERVICE_CONTROL_STOP, &status);
				do
				{
					Sleep(50);
					num++;
					QueryServiceStatus(hs, &status);
				}while (status.dwCurrentState != SERVICE_STOPPED || num>80);
			}

			if(num>80)
			{
				printf("停止服务失败\n");
			}
			else
			{
				printf("停止服务成功\n");
			}
			CloseServiceHandle(hs);
		}
		CloseServiceHandle(schSCManager);
	}

}
void unloadDvr()//卸载
{
	SC_HANDLE schSCManager;
	SC_HANDLE hs;
	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(schSCManager)
	{
		hs=OpenService(schSCManager, "babaHelloDDKk", SERVICE_ALL_ACCESS); //打开服务
		if (hs)
		{
			if (!DeleteService(hs))
			{
				printf("删除服务失败\n");
			}
			else
			{
				printf("已删除服务\n");
			}

			CloseServiceHandle(hs);//释放完后可完服务可从服务表中消失 释放前是已禁止状态
		}
		CloseServiceHandle(schSCManager);
	} 

}