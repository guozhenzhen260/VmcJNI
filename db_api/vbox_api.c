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
        EV_LOGD("VB-Send[%d]:%s\n",len,buf);
    }
    else{
        EV_LOGD("VB-Recv[%d]:%s\n",len,buf);
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


int VBOX_getSetup(int32 port)
{
    VBOX_MSG *msg = &vboxMsg;
    msg->mt = VBOX_GET_SETUP;
    msg->F7 = 0;
    msg->datalen = 0;
    return VBOX_sendMsg(port,msg);
}



int VBOX_getSetup1(int32 port)
{
    VBOX_MSG *msg = &vboxMsg;
    msg->mt = VBOX_GET_SETUP;
    msg->F7 = 0;
    msg->datalen = 0;
    return VBOX_sendMsg(port,msg);
}




#if 0

//出货指令
uint8 LIFT_vmcVendingReq(uint8 bin,uint8 row,uint8 column)
{
	uint8 res;
	ST_LIFT_MSG *msg = &liftMsg;
	msg->addr = 0x40;
	msg->bin = bin;
	msg->cmd = VMC_VENDING_REQ;
	msg->data[0] = row;
	msg->data[1] = column;
	msg->datalen = 2;
	res = LIFT_send(msg);
	if(res == 1){
		return  msg->recvbuf[I_MT] == GCC_VENDING_ACK ?  1 : 2;
	}
	else{ 
		return res;
	}
}

//0x1F 通信故障 0x10 正常 0x11 整机忙 0x12故障
uint8 LIFT_vmcStatusReq(uint8 bin)
{
	uint8 res,status;
	ST_LIFT_MSG *msg = &liftMsg;
	msg->addr = 0x40;
	msg->bin = bin;
	msg->cmd = VMC_STATUS_REQ;
	msg->datalen = 0;
	
	res =  LIFT_send(msg);
	if(res == 1 && msg->recvbuf[I_MT] == GCC_STATUS_ACK){
		//stLiftTable.Error_OVERALLUINT = msg->recvbuf[5];
		status = msg->recvbuf[5];
		if(status & 0x01){
			return LIFT_STATUS_BUSY; //忙
		}
		if(status & 0x80){ //货没取走
			return LIFT_VENDOUT_GOODS_NOT_TAKE;
		}
		
		if(msg->recvbuf[6] == 0x00){
			return LIFT_STATUS_NORMAL; //正常
		}
		else{
			return LIFT_STATUS_FAULT; //故障
		}
	}
	else{
		return LIFT_STATUS_COM_ERR; //通信故障
	}
}







uint8 LIFT_vmcReset(uint8 bin)
{
	uint8 res;
	ST_LIFT_MSG *msg = &liftMsg;
	msg->addr = 0x40;
	msg->bin = bin;
	msg->cmd = VMC_RESET_REQ;
	msg->datalen = 0;
	
	res =  LIFT_send(msg);
	if(res == 1 && msg->recvbuf[I_MT] == GCC_RESET_ACK){
		return 1;
	}
	else{
		return 0;
	}
}




//返回0:失败，1：成功，2：数据错误 3：无货 4：卡货 5：取货门未开启 6：货物未取走 7：未定义错误	0xff：通信失败
// 0 未接受 1成功 0x88正在出货
uint8 LIFT_vmcVedingResult(uint8 bin)
{
	uint8 res,temp;
	ST_LIFT_MSG *msg = &liftMsg;
	msg->addr = 0x40;
	msg->bin = bin;
	msg->cmd = VMC_VENDINGRESULT_REQ;
	msg->datalen = 0;
	
	res =  LIFT_send(msg);
	if(res == 1 && msg->recvbuf[I_MT] == GCC_VENDINGRESULT_ACK){
		if(msg->recvbuf[I_DATA] != 0x00){
			return LIFT_VENDOUT_VENDING;
		}
		if(msg->recvbuf[I_DATA + 1] == 0x00){
			return LIFT_VENDOUT_SUC;
		}

		temp = msg->recvbuf[I_DATA + 2];
		if(temp == 0x00){
			return LIFT_VENDOUT_SUC;
		}
		else if(temp == 0x01){
			return LIFT_VENDOUT_DATAERR;
		}
		else if(temp == 0x02){
			return LIFT_VENDOUT_EMPTY;
		}
		else if(temp == 0x03){
			return LIFT_VENDOUT_STUCK;
		}
		else if(temp == 0x04){
			return LIFT_VNEDOUT_DOOR_NOT_OPEN;
		}
		else if(temp == 0x05){
			return LIFT_VENDOUT_GOODS_NOT_TAKE;
		}
		else if(temp == 0x06){
			return LIFT_VENDOUT_OTHER_FAULT;
		}
		else if(temp == 0x07){
			return LIFT_VENDOUT_OTHER_FAULT;
		}
		else{
			return LIFT_VENDOUT_OTHER_FAULT;
		}
	}
	else{
		return 0;
	}
}


uint8 LIFT_vmcVendingResultByTime(uint8 bin,unsigned int ms)
{
	uint8 res;
	while(ms > 0){
		res = LIFT_vmcVedingResult(bin);
		if(res > 0){
			return res;
		}
		else{
			EV_msleep(1000);
			ms--;
		}
	}
	return 0;
}


uint8 LIFT_vmcOpenLight(uint8 bin)
{
	uint8 res;
	ST_LIFT_MSG *msg = &liftMsg;
	msg->addr = 0x40;
	msg->bin = bin;
	msg->cmd = VMC_OPENLIGHT_REQ;
	msg->datalen = 0;
	
	res =  LIFT_send(msg);
	if(res == 1 && msg->recvbuf[I_MT] == GCC_OPENLIGHT_ACK){
		return 1;
	}
	else{
		return 0;
	}


}

uint8 LIFT_vmcCloseLight(uint8 bin)
{
	uint8 res;
	ST_LIFT_MSG *msg = &liftMsg;
	msg->addr = 0x40;
	msg->bin = bin;
	msg->cmd = VMC_CLOSELIGHT_REQ;
	msg->datalen = 0;
	
	res =  LIFT_send(msg);
	if(res == 1 && msg->recvbuf[I_MT] == GCC_CLOSELIGHT_ACK){
		return 1;
	}
	else{
		return 0;
	}
}


uint8 LIFT_vmcChuchou(uint8 bin,uint8 flag)
{
	uint8 res;
	ST_LIFT_MSG *msg = &liftMsg;
	msg->addr = 0x40;
	msg->bin = bin;
	msg->cmd = VMC_CHUCHOU_REQ;
	msg->data[0] = flag;
	msg->datalen = 1;
	
	res =  LIFT_send(msg);
	if(res == 1 && msg->recvbuf[I_MT] == GCC_CHUCHOU_ACK){
		return 1;
	}
	else{
		return 0;
	}
}





//返回0:失败，1：成功，2：数据错误 3：无货 4：卡货 5：取货门未开启 6：货物未取走 7：未定义错误	0xff：通信失败
uint8 LIFT_vendoutReq(uint8 bin,uint8 row,uint8 column)
{
	uint8 res,res1,flow,err,tradeResult,vendoutOk,i;
	int32 timeout;
	timeout = 90;flow = 0;
	while(timeout > 0){
		res = LIFT_vmcStatusReq(bin);
		if(res == LIFT_STATUS_NORMAL){
			flow = 1;
			break;
		}
		else{
			if(res != LIFT_STATUS_COM_ERR){
				EV_msleep(1000);
				timeout--;
			}			
		}
	}
	if(flow == 0){ //有问题
		return res;
	}


	// 2 出货
	for(i = 0;i < 3;i++){
		EV_msleep(500);
		res = LIFT_vmcVendingReq(bin,row,column);
		if(res != 1){
			EV_msleep(50);
			res1 = LIFT_vmcVedingResult(bin);
			if(res1 == LIFT_VENDOUT_VENDING){
				vendoutOk = 1;
				break;
			}
		}
		else{
			vendoutOk = 1;
			break;
		}
	}

	if(vendoutOk != 1){
		return LIFT_VENDOUT_COM_ERR;
	}

	// 3查询
	vendoutOk = 0;
	tradeResult = LIFT_VENDOUT_FAIL;
	timeout = 240;
	while(timeout > 0){
		res = LIFT_vmcVedingResult(bin);//检测出货结果
		if(res > 0 && res != LIFT_VENDOUT_VENDING){ //出货结果
			return res;
		}
		else{
			EV_msleep(1000);
			timeout--;
		}
	}

	return tradeResult;
}





uint8 LIFT_vendout(ST_COL_OPEN_REQ *req,ST_COL_OPEN_RPT *rpt)
{
	int32 ret;
	ST_LIFT_MSG *msg = &liftMsg;
	uint8 row,col;
    if(req == NULL || rpt == NULL){
        EV_LOGE("LIFT_vendout:s=%x,r=%x",(unsigned int)req,(unsigned int)rpt);
        return 0;
    }

    if(req->addr <= 0){
        EV_LOGE("LIFT_vendout:req->addr = %d",req->addr);
        return 0;
    }

    if(req->no <= 0){
        EV_LOGE("LIFT_vendout:req->no = %d",req->no);
        return 0;
    }

    memset((void *)rpt,0,sizeof(ST_COL_OPEN_RPT));
    rpt->addr = req->addr;
    rpt->fd = req->fd;
    rpt->no = req->no;

    msg->port  = rpt->fd;
    msg->addr = rpt->addr;
	msg->bin = rpt->addr;

	row = rpt->no / 10;
	col = rpt->no % 10;
	ret = LIFT_vendoutReq(msg->bin,row,col);
	
   	rpt->is_success = 1;
    rpt->result = ret;

	return 1;

}
#endif



