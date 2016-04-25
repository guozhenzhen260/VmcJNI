#include "lift_api.h"
#include "ev_config.h"
#include "db_serial.h"
#include "timer.h"
#include "yoc_serialport.h"



static unsigned char sendbuf[64] = {0};
//static unsigned char sendlen = 0;
static unsigned char recvbuf[64] = {0};
static unsigned char index = 0;
static unsigned char recvlen = 0;


const static unsigned char I_SF = 0;
const static unsigned char I_LEN = 1;
const static unsigned char I_ADDR = 2;
const static unsigned char I_VER = 3;
const static unsigned char I_MT = 4;
const static unsigned char I_DATA = 5;


typedef struct{
	unsigned char addr;
	unsigned char cmd;
	unsigned char bin;
	unsigned char row;
	unsigned char column;
	unsigned char *recvbuf;
	unsigned char *sendbuf;
	unsigned char recvlen;
	unsigned char sendlen;
	unsigned char data[8];
	unsigned char datalen;

}ST_LIFT_MSG;

ST_LIFT_MSG liftMsg;



/*********************************************************************************************************
** Function name:     	crc16
** Descriptions:	    CRC校验和
** input parameters:    msg需要检验的数据;len数据长度
** output parameters:   无
** Returned value:      CRC检验结果
*********************************************************************************************************/
static uint16 crc16(uint8 *msg, uint16 len) 
{
    uint16 i, j;
    uint16 crc = 0;
    uint16 current = 0;
    for(i=0;i<len;i++) 
    {
        current = msg[i] << 8;
        for(j=0;j<8;j++) 
        {
            if((short)(crc^current)<0)
                crc = (crc<<1)^0x1021;
            else 
                crc <<= 1; 
            current <<= 1; 
        }
    }
    return crc;
}



static void LIFT_clear(void)
{
	index = 0;recvlen = 0;
	memset(recvbuf,0,sizeof(recvbuf));
}







