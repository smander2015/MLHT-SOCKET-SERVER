#pragma once
#include<winsock.h>


#pragma comment (lib, "ws2_32")			//socket库文件


#define MLHT_HEADER_LENGTH 12			//MLHT协议HEADER长度
#define MLHT_MAX_LENGTH 1500			//MLHT协议报文最大长度
#define MLHT_SLICE_FILE_LENGTH (MLHT_MAX_LENGTH-MLHT_HEADER_LENGTH)


// MLHT协议报文类型

// MTU_TYPE HEADER STRUCTURE
// SYN ACK FIN  *  PROCESS FILE  RC  SHELL TOTAL:2BYTE
//  1   1   1   1     2     2    4     4       BIT

#define MLHT_MTU_TYPE_SYN			0x8000			//SYN
#define MLHT_MTU_TYPE_ACK			0x4000			//ACK
#define MLHT_MTU_TYPE_FIN			0x2000			//FIN

#define MLHT_MTU_TYPE_PROCESS_HIDE  0x0800			//PROCESS_HIDE
#define MLHT_MTU_TYPE_PROCESS_SHOW  0x0400			//PROCESS_SHOW

#define MLHT_MTU_TYPE_FILE_HIDE		0x0200			//FILE_HIDE
#define MLHT_MTU_TYPE_FILE_SHOW		0x0100			//FILE_SHOW

#define MLHT_MTU_TYPE_RC_START		0x0010			//REMOTE_CONTROL_START
#define MLHT_MTU_TYPE_RC_END		0x0020			//REMOTE_CONTROL_END
#define MLHT_MTU_TYPE_RC_SCREEN     0x0030			//REMOTE_CONTROL_SCREEN
#define MLHT_MTU_TYPE_RC_MOUSE_OPERATE	  0x0040	//REMOTE_CONTROL_MOUSE_OPERATE
#define MLHT_MTU_TYPE_RC_KEYBROAD_OPERATE 0x0050	//REMOTE_CONTROL_KEYBROAD_OPERATE

#define MLHT_MTU_TYPE_SHELL_LS	0x0001				//SHELL_LS
#define MLHT_MTU_TYPE_SHELL_DIR 0x0001				//SHELL_DIR
#define MLHT_MTU_TYPE_SHELL_CD 0x0002				//SHELL_CD
#define MLHT_MTU_TYPE_SHELL_MKDIR 0x0003			//SHELL_MKDIR
#define MLHT_MTU_TYPE_SHELL_RMDIR 0x0004			//SHELL_RMDIR
#define MLHT_MTU_TYPE_SHELL_RM 0x0005				//SHELL_RM
#define MLHT_MTU_TYPE_SHELL_UPLOAD 0x0006			//SHELL_UPLOAD
#define MLHT_MTU_TYPE_SHELL_DOWNLOAD 0x0007			//SHELL_DOWNLOAD


#define BYTE_SIZE 0x100
#define WORD_SIZE 0x10000

#define MAX_FILE_SIZE WORD_SIZE*MLHT_SLICE_FILE_LENGTH


class MLHT
{
private://协议内容
	
	WORD ID;					//ID号 2byte

    WORD mtu_type;				//MTU操作类型 2byte

    WORD slice_num;				//第几个分片 2byte
    WORD slice_sum;				//共几个分片 2byte
	
	WORD total_bytes;			//总共字节数 2byte,表示长度可达0-65535字节

	WORD checksum;				//校验和 2byte

	BYTE slice_file_content[MLHT_SLICE_FILE_LENGTH];	//报文内容 不定长

public:
	MLHT(){
		ID = 0;
		mtu_type = 0;
		slice_num = slice_sum = 1;
		total_bytes = MLHT_HEADER_LENGTH;
		checksum = countchecksum();
	}

	//初始化无文件内容类报文
	MLHT(WORD _ID,WORD _mtu_type){
		ID = _ID;
		mtu_type = _mtu_type;
		slice_num = slice_sum = 1;
		total_bytes = MLHT_HEADER_LENGTH;
		checksum = countchecksum();
	}

	//初始化含文件内容类报文
	MLHT(WORD _ID,WORD _mtu_type,WORD _slice_num,WORD _slice_sum,BYTE*_slice_file_content,int _slice_file_length){
		ID = _ID;
		mtu_type = _mtu_type;
		slice_num = _slice_num;
		slice_sum = _slice_sum;
		total_bytes = MLHT_HEADER_LENGTH + _slice_file_length;
		memcpy(slice_file_content,_slice_file_content,_slice_file_length);
		checksum = countchecksum();
	}

	//获得报文长度
	WORD GetPacketLength(){
		return total_bytes;
	}

	//获得报文ID
	WORD GetPacketId(){
		return ID;
	}

	//获得报文TYPE
	WORD GetPacketType(){
		return mtu_type;
	}

	//获得报文分片序列号
	WORD GetPacketSliceNum(){
		return slice_num;
	}
	
	//获得报文分片总数
	WORD GetPacketSliceSum(){
		return slice_sum;
	}

	//获得报文内容
	BYTE* GetPacketContent(){
		return slice_file_content;
	}

	//是否为最后一个分片
	bool IsLastPacket(){
		return slice_num == slice_sum;
	}

	//计算校验和
	WORD countchecksum(){
		int sum = 0;
		sum += ID;
		sum += mtu_type;
		sum += slice_num;
		sum += slice_sum;
		sum += total_bytes;
		int numoffilecont = (total_bytes - MLHT_HEADER_LENGTH)/2;
		for (int i = 0; i < numoffilecont; i++)
			sum += slice_file_content[2*i]*BYTE_SIZE+slice_file_content[2*i+1];
		if((total_bytes - MLHT_HEADER_LENGTH)%2)
			sum += slice_file_content[total_bytes - MLHT_HEADER_LENGTH- 1]*BYTE_SIZE;
		while(sum > WORD_SIZE)
			sum = sum / WORD_SIZE + sum % WORD_SIZE;
		return 0xffff - sum;
	}

	//解构已接收的报文并检测是否为合法报文
	bool PacketAccept(BYTE* packet){
		ID = packet[0]*BYTE_SIZE+packet[1];
		mtu_type = packet[2]*BYTE_SIZE+packet[3];
		slice_num = packet[4]*BYTE_SIZE+packet[5];
		slice_sum = packet[6]*BYTE_SIZE+packet[7];
		total_bytes = packet[8]*BYTE_SIZE+packet[9];
		checksum = packet[10]*BYTE_SIZE+packet[11];
		memcpy(slice_file_content,&packet[12],total_bytes - MLHT_HEADER_LENGTH);
		return PacketCheck();
	}

	//拼接成要发送的报文
	void PacketSplice(BYTE* packet){
		packet[0] = ID/BYTE_SIZE;
		packet[1] = ID%BYTE_SIZE;
		packet[2] = mtu_type/BYTE_SIZE;
		packet[3] = mtu_type%BYTE_SIZE;
		packet[4] = slice_num/BYTE_SIZE;
		packet[5] = slice_num%BYTE_SIZE;
		packet[6] = slice_sum/BYTE_SIZE;
		packet[7] = slice_sum%BYTE_SIZE;
		packet[8] = total_bytes/BYTE_SIZE;
		packet[9] = total_bytes%BYTE_SIZE;
		packet[10] = checksum/BYTE_SIZE;
		packet[11] = checksum%BYTE_SIZE;
		memcpy(&packet[12],slice_file_content,total_bytes - MLHT_HEADER_LENGTH);
	}

	//检测是否为合法报文
	bool PacketCheck(){
		if(checksum == countchecksum())
			return true;
		else
			return false;
	}

	//解析报文内容
	int PacketAnalyze_SYNACKFIN(){
		//检测是否为SYN,ACK,FIN
		if(mtu_type & MLHT_MTU_TYPE_SYN)
			return MLHT_MTU_TYPE_SYN;
		if(mtu_type & MLHT_MTU_TYPE_ACK)
			return MLHT_MTU_TYPE_ACK;
		if(mtu_type & MLHT_MTU_TYPE_FIN)
			return MLHT_MTU_TYPE_FIN;
		return 0;
	}

	int PacketAnalyze_PROCESS(){
		//检测是否为PROCESS操作
		if(mtu_type & MLHT_MTU_TYPE_PROCESS_HIDE)
			return MLHT_MTU_TYPE_PROCESS_HIDE;
		if(mtu_type & MLHT_MTU_TYPE_PROCESS_SHOW)
			return MLHT_MTU_TYPE_PROCESS_SHOW;
		return 0;
	}
		
	int PacketAnalyze_FILE(){
		//检测是否为FILE操作
		if(mtu_type & MLHT_MTU_TYPE_FILE_HIDE)
			return MLHT_MTU_TYPE_FILE_HIDE;
		if(mtu_type & MLHT_MTU_TYPE_FILE_SHOW)
			return MLHT_MTU_TYPE_FILE_SHOW;
		return 0;
	}

	int PacketAnalyze_RC(){
		//检测为哪种RC操作
		switch(mtu_type & 0x00f0){
		case MLHT_MTU_TYPE_RC_START:
			return MLHT_MTU_TYPE_RC_START;
		case MLHT_MTU_TYPE_RC_END:
			return MLHT_MTU_TYPE_RC_END;
		case MLHT_MTU_TYPE_RC_SCREEN:
			return MLHT_MTU_TYPE_RC_SCREEN;
		case MLHT_MTU_TYPE_RC_MOUSE_OPERATE:
			return MLHT_MTU_TYPE_RC_MOUSE_OPERATE;
		case MLHT_MTU_TYPE_RC_KEYBROAD_OPERATE:
			return MLHT_MTU_TYPE_RC_KEYBROAD_OPERATE;
		default:
			return 0;
		}
	}
	
	int PacketAnalyze_SHELL(){
		//检测为哪种SHELL操作
		switch(mtu_type & 0x000f){
		case MLHT_MTU_TYPE_SHELL_LS:
			return MLHT_MTU_TYPE_SHELL_LS;
		case MLHT_MTU_TYPE_SHELL_CD:
			return MLHT_MTU_TYPE_SHELL_CD;
		case MLHT_MTU_TYPE_SHELL_MKDIR:
			return MLHT_MTU_TYPE_SHELL_MKDIR;
		case MLHT_MTU_TYPE_SHELL_RMDIR:
			return MLHT_MTU_TYPE_SHELL_RMDIR;
		case MLHT_MTU_TYPE_SHELL_RM:
			return MLHT_MTU_TYPE_SHELL_RM;
		case MLHT_MTU_TYPE_SHELL_UPLOAD:
			return MLHT_MTU_TYPE_SHELL_UPLOAD;
		case MLHT_MTU_TYPE_SHELL_DOWNLOAD:
			return MLHT_MTU_TYPE_SHELL_DOWNLOAD;
		default:
			return 0;
		}
	}

};