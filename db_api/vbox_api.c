#include "vbox_api.h"
#include "ev_config.h"
#include "db_serial.h"
#include "timer.h"
#include "yoc_serialport.h"

const static uint8 HEAD_SF = 0xE7;
const static uint8 HEAD_LEN = 5;
const static uint8 VER_F0_1 = 0x41;
const static uint8 VER_F0_0 = 0x40;
const static uint8 CRC_LEN = 2;

const static uint8 I_SF = 0;
const static uint8 I_LEN = 1;
const static uint8 I_VER = 2;
const static uint8 I_MT = 3;
const static uint8 I_SN = 4;
const static uint8 I_DATA = 5;





VBOX_MSG vboxMsg;






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



static void VBOX_LOG(uint8 type,uint8 *data,uint8 len)
{
    uint8 i,buf[512] = {0};
    for(i = 0;i < len;i++){
        sprintf((char *)&buf[i * 3],"%02x ",data[i]);
    }
    if(type == 1){
        EV_LOGD("VB-Send[%d]:%s",len,buf);
    }
    else{
        EV_LOGI("VB-Recv[%d]:%s\n",len,buf);
    }
}



static void VBOX_clear(void)
{
    memset(vboxMsg.recvbuf,0,sizeof(vboxMsg.recvbuf));
    vboxMsg.recvlen = 0;
}


static uint8 VBOX_recv(VBOX_MSG *msg,uint32 timeout)
{
	uint8 ch,index;
	uint16 crc;
	uint8 len = 0;
	int32 r;
    uint8 *recvbuf;
	index = 0;recvbuf = msg->recvbuf;
	while(timeout > 0){
        r = DB_getCh(msg->port,(char *)&ch);
		if(r <= 0){
			EV_msleep(50);
			timeout = (timeout <= 50) ? 0 : timeout - 50;
			continue;
		}
		if(index == 0){
            if(ch == HEAD_SF){
				recvbuf[index++] = ch;
                timeout += 200;
			}
			else{
				index = 0;
			}
		}
		else if(index == 1){
			len = ch;
			recvbuf[index++] = ch;
		}
		else{
			if(index >= (len + 1)){
				recvbuf[index++] = ch;
				crc  = crc16(recvbuf,len); 
				msg->recvlen = index;
				if(crc == INTEG16(recvbuf[len], recvbuf[len + 1])){
                    msg->mt = msg->recvbuf[I_MT];
                    msg->sn = msg->recvbuf[I_SN];
                    msg->ver = msg->recvbuf[I_VER] & 0x07;
                    msg->F7 = msg->recvbuf[I_VER] & (0x01 << 7) ? 1 : 0;
					return 1;
				}
				else{
					return 2;
				}
			}
			else{
				recvbuf[index++] = ch;
			}
		}
	}
	msg->recvlen = index;
	return 0;
}

static uint8 VBOX_send(VBOX_MSG *msg)
{
    uint8 i,res,in = 0;
    uint16 crc = 0x0000;
    uint8 *sendbuf;
    in = 0;sendbuf = msg->sendbuf;
    sendbuf[in++] = HEAD_SF;
    sendbuf[in++] = 0x00;
    sendbuf[in++] = (msg->F7 << 7) | (msg->ver & 0x07);
    sendbuf[in++] = msg->mt;
    sendbuf[in++] = msg->sn;

    for(i = 0;i < msg->datalen;i++){
        sendbuf[in++] = msg->data[i];
    }
    sendbuf[I_LEN] = in;
    crc	= crc16(sendbuf,in);
    sendbuf[in++] = HUINT16(crc);
    sendbuf[in++] = LUINT16(crc);
    //yserial_clear(msg->port);
    yserial_write(msg->port,(const char *)sendbuf,in);
    VBOX_LOG(1,sendbuf,in);
    return 1;
}


VBOX_MSG *VBOX_getMsg(void)
{
    return &vboxMsg;
}


VBOX_MSG *VBOX_readMsg(int32 port,uint32 timeout)
{
    VBOX_MSG *msg = &vboxMsg;

    msg->port = port;
    msg->res = VBOX_recv(msg,timeout);
    if(msg->recvlen > 0){
        VBOX_LOG(2,msg->recvbuf,msg->recvlen);
    }
    return msg;
}


int VBOX_sendMsg(int32 port,VBOX_MSG *msg)
{
    uint8 res;
    msg->port = port;
    res = VBOX_send(msg);
    return (int )res;
}


int VBOX_sendAck(int32 port)
{
    VBOX_MSG *msg = &vboxMsg;
    msg->mt = VBOX_ACK;
    msg->F7 = 0;
    msg->datalen = 0;
    return VBOX_sendMsg(port,msg);
}

int VBOX_sendNak(int32 port)
{
    VBOX_MSG *msg = &vboxMsg;
    msg->mt = VBOX_NAK;
    msg->F7 = 0;
    msg->datalen = 0;
    return VBOX_sendMsg(port,msg);
}


int VBOX_getSetup(int32 port)
{
    VBOX_MSG *msg = &vboxMsg;
    msg->mt = VBOX_GET_SETUP;
    msg->F7 = 0;
    msg->datalen = 0;
    return VBOX_sendMsg(port,msg);
}



int VBOX_getHuoDao(int32 port,int32 device)
{
    unsigned char in;
    VBOX_MSG *msg = &vboxMsg;
    in = 0;
    msg->mt = VBOX_GET_HUODAO;
    msg->F7 = 0;
    msg->datalen = 1;
    msg->data[in++] = (uint8)device;
    return VBOX_sendMsg(port,msg);
}


int VBOX_getStatus(int32 port)
{
    unsigned char in;
    VBOX_MSG *msg = &vboxMsg;
    in = 0;
    msg->mt = VBOX_GET_STATUS;
    msg->F7 = 0;
    msg->datalen = 0;
    return VBOX_sendMsg(port,msg);
}



int VBOX_getInfo(int32 port,int32 type)
{
    unsigned char in;
    VBOX_MSG *msg = &vboxMsg;
    in = 0;
    msg->mt = VBOX_GET_INFO;
    msg->F7 = 0;
    msg->datalen = 1;
    msg->data[in++] = (uint8)type;
    return VBOX_sendMsg(port,msg);
}


int VBOX_resetInd(int32 port,int32 dt)
{
    unsigned char in;
    VBOX_MSG *msg = &vboxMsg;
    in = 0;
    msg->mt = VBOX_RESET_IND;
    msg->F7 = 0;
    msg->data[in++] = dt & 0xFF;
    msg->datalen = in;
    return VBOX_sendMsg(port,msg);
}


int VBOX_controlInd(int32 port,int32 type,int32 value)
{
    unsigned char in;
    VBOX_MSG *msg = &vboxMsg;
    in = 0;
    msg->mt = VBOX_CONTROL_IND;
    msg->F7 = 1;
    msg->data[in++] = type & 0xFF;
    msg->data[in++] = value & 0xFF;
    msg->datalen = in;
    return VBOX_sendMsg(port,msg);
}


int VBOX_huodaoInd(int32 port,uint8 device,uint8 *buf,uint8 len)
{
    unsigned char in,i;
    VBOX_MSG *msg = &vboxMsg;
    in = 0;
    msg->mt = VBOX_HUODAO_IND;
    msg->F7 = 1;
    msg->data[in++] = device & 0xFF;
    for(i = 0;i < len;i++){
        msg->data[in++] = buf[i];
    }
    msg->datalen = in;
    return VBOX_sendMsg(port,msg);
}


int VBOX_positionInd(int32 port,uint8 device,uint8 *buf,uint8 len)
{
    unsigned char in,i;
    VBOX_MSG *msg = &vboxMsg;
    in = 0;
    msg->mt = VBOX_POSITION_IND;
    msg->F7 = 1;
    msg->data[in++] = device & 0xFF;
    for(i = 0;i < len;i++){
        msg->data[in++] = buf[i];
    }
    msg->datalen = in;
    return VBOX_sendMsg(port,msg);
}


int VBOX_payoutInd(int32 port,uint8 device,uint16 value,uint8 type)
{
    unsigned char in;
    VBOX_MSG *msg = &vboxMsg;
    in = 0;
    msg->mt = VBOX_PAYOUT_IND;
    msg->F7 = 1;
    msg->data[in++] = device & 0xFF;
    msg->data[in++] = HUINT16(value);
    msg->data[in++] = LUINT16(value);
    msg->data[in++] = type;
    msg->datalen = in;
    return VBOX_sendMsg(port,msg);
}


int VBOX_costInd(int32 port,uint8 device,uint16 value,uint8 type)
{
    unsigned char in;
    VBOX_MSG *msg = &vboxMsg;
    in = 0;
    msg->mt = VBOX_COST_IND;
    msg->F7 = 1;
    msg->data[in++] = device & 0xFF;
    msg->data[in++] = HUINT16(value);
    msg->data[in++] = LUINT16(value);
    msg->data[in++] = type;
    msg->datalen = in;
    return VBOX_sendMsg(port,msg);
}


int VBOX_salePriceInd(int32 port,uint8 device,uint16 *buf,uint8 len)
{
    unsigned char in,i;
    VBOX_MSG *msg = &vboxMsg;
    in = 0;
    msg->mt = VBOX_SALEPRICE_IND;
    msg->F7 = 1;
    msg->data[in++] = device & 0xFF;
    for(i = 0;i < len;i++){
        msg->data[in++] = HUINT16(buf[i]);
        msg->data[in++] = LUINT16(buf[i]);
    }
    msg->datalen = in;
    return VBOX_sendMsg(port,msg);
}

int VBOX_huodaoSetInd(int32 port,uint8 device,uint8 *buf,uint8 len)
{
    unsigned char in,i;
    VBOX_MSG *msg = &vboxMsg;
    in = 0;
    msg->mt = VBOX_HUODAO_SET_IND;
    msg->F7 = 1;
    msg->data[in++] = device & 0xFF;
    for(i = 0;i < len;i++){
        msg->data[in++] = buf[i];
    }
    msg->datalen = in;
    return VBOX_sendMsg(port,msg);
}


int VBOX_vendoutInd(int32 port,int32 device,int32 method,int32 id,int32 type,int32 cost)
{
    unsigned char in;
    VBOX_MSG *msg = &vboxMsg;
    in = 0;
    msg->mt = VBOX_VENDOUT_IND;
    msg->F7 = 1;
    msg->data[in++] = (uint8)device;
    msg->data[in++] = (uint8)method;
    msg->data[in++] = (uint8)id;
    msg->data[in++] = (uint8)type;
    msg->data[in++] = HUINT16(cost);
    msg->data[in++] = LUINT16(cost);

    msg->datalen = in;
    return VBOX_sendMsg(port,msg);
}
