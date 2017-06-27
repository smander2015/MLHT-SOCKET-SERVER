#include<stdio.h>


extern MLHT_SERVER* ms_rc;  //MLHT 服务器


void Rc_Working(){
	
	switch(ms_rc->p_get->PacketAnalyze_RC()){
	case MLHT_MTU_TYPE_RC_START:
		return;
	case MLHT_MTU_TYPE_RC_END:
		return;
	case MLHT_MTU_TYPE_RC_SCREEN:
		switch(ms_rc->GetScreenshoot())
		{            
		case PACKET_ACCEPT_SUCCESS:
			printf("Rc Server: Screenshoot success.\n");
			break;
		case FILE_WRITE_FAIL:
			printf("Rc Server: Write file fail.\n");
			ms_rc->SendFINPacket();
			printf("Rc Server: Send FIN packet back...\n");
			break;
		case FILE_OPEN_FAIL:
			printf("Rc Server: Open file fail.\n");
			ms_rc->SendFINPacket();
			printf("Rc Server: Send FIN packet back...\n");
			break;
		case PACKET_ACCEPT_FAIL:
			printf("Rc Server: Packet accept fail.\n");
			ms_rc->SendFINPacket();
			printf("Rc Server: Send FIN packet back...\n");
			break;
		default:
			printf("Rc Server: Client is blocked\n");
			break;
		}
		return;
	case MLHT_MTU_TYPE_RC_MOUSE_OPERATE:
		ms_rc->GetMouseOperate();
		return;
	case MLHT_MTU_TYPE_RC_KEYBROAD_OPERATE:
		ms_rc->GetKeybroadOperate();
		return;
	default:
		break;
	}

}


void Thread_RemoteDesktop(){
	/*while(TRUE){
	SceenShoot();
	Sleep(1000);
	}*/
	int ret;

	ms_rc = new MLHT_SERVER(9001);

	while(true)
	{
		printf("Rc Server: Wait client to link:\n");
		ms_rc->WaitForClient();
		printf("Rc Server: Linked by client\nSend ACK packet back...\n");

		do{
			ret = ms_rc->WaitReturnPacket();//一直接收客户端socket的send操作

			switch(ret)
			{            
			case PACKET_ACCEPT_SUCCESS:
				ms_rc->SendACKPacket();
				Rc_Working();
				break;
			case PACKET_ACCEPT_FAIL:
				printf("Rc Server: Get a bad packet from client.\n");
				ms_rc->SendFINPacket();
				printf("Rc Server: Send FIN packet back...\n");
				break;
			default:
				printf("Rc Server: Client is blocked\n");
				break;
			}
		}while(ret!=SOCKET_ERROR);
	}
}