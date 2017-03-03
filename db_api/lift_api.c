#include "lift_api.h"
#include "ev_config.h"
#include "db_serial.h"
#include "timer.h"
#include "yoc_serialport.h"



const static uint8 I_SF = 0;
const static uint8 I_LEN = 1;
const static uint8 I_ADDR = 2;
const static uint8 I_VER = 3;
const static uint8 I_MT = 4;
const static uint8 I_DATA = 5;


typedef struct{
	int32 port;
	uint8 addr;
	uint8 cmd;
	uint8 bin;
	uint8 row;
	uint8 column;
	uint8 sendbuf[256];
	uint8 sendlen;
	uint8 recvbuf[256];
	uint8 recvlen;
	uint8 data[8];
	uint8 datalen;

}ST_LIFT_MSG;

ST_LIFT_MSG liftMsg;



#define LIFT_STATUS_COM_ERR		0x1F
#define LIFT_STATUS_FAULT		0x12
#define LIFT_STATUS_BUSY		0x11
#define LIFT_STATUS_NORMAL		0x10



#define LIFT_VENDOUT_COM_ERR			0x1F		//通信故障
#define LIFT_VENDOUT_FAULT				0x12		//升降机故障
#define LIFT_VENDOUT_BUSY				0x11		//升降机忙
#define LIFT_VENDOUT_FAIL				0			//出货失败 通信失败
#define LIFT_VENDOUT_SUC				1			//出货成功
#define LIFT_VENDOUT_DATAERR			2			//数据错误
#define LIFT_VENDOUT_EMPTY				3			//无货
#define LIFT_VENDOUT_STUCK				4  			//卡货
#define LIFT_VNEDOUT_DOOR_NOT_OPEN		5			//取货门未开启
#define LIFT_VENDOUT_GOODS_NOT_TAKE		6			//货物未取走
#define LIFT_VENDOUT_OTHER_FAULT		7			//其他故障
#define LIFT_VENDOUT_VENDING			0x88		//正在出货




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



static void LIFT_LOG(uint8 type,uint8 *data,uint8 len)
{
    uint8 i,buf[512] = {0};
    for(i = 0;i < len;i++){
        sprintf((char *)&buf[i * 3],"%02x ",data[i]);
    }
    if(type == 1){
        EV_LOGD("LF-Send[%d]:%s",len,buf);
    }
    else{
        EV_LOGI("LF-Recv[%d]:%s\n",len,buf);
    }
}



static void LIFT_clear(void)
{
	memset(liftMsg.recvbuf,0,sizeof(liftMsg.recvbuf));
	liftMsg.recvlen = 0;
}


static uint8 LIFT_recv(ST_LIFT_MSG *msg,uint32 timeout){
	uint8 ch,index;
	uint16 crc;
	uint8 len = 0;
	int32 r;
	uint8 *recvbuf;
	index = 0;recvbuf = msg->recvbuf;

	while(timeout > 0){
        r = DB_getCh(msg->port,(char *)&ch);
        //EV_LOGD("r=%02x,ch=%02x\n", r, ch);
		if(r <= 0){
			EV_msleep(50);
			timeout = (timeout <= 50) ? 0 : timeout - 50;
			continue;
		}
		if(index == 0){
			if(ch == 0xC8){
				recvbuf[index++] = ch;
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


static uint8 LIFT_send(ST_LIFT_MSG *msg)
{
	uint8 i,res,in = 0;
	uint16 crc = 0x0000;
	uint8 *sendbuf;
	
	in = 0;sendbuf = msg->sendbuf;
	sendbuf[in++] = 0xC7;	
	sendbuf[in++] = 0x00;
	sendbuf[in++] = msg->addr;
	sendbuf[in++] = 0x00;
	sendbuf[in++] = msg->cmd;
	
	for(i = 0;i < msg->datalen;i++){
		sendbuf[in++] = msg->data[i];
	}
	sendbuf[I_LEN] = in;
	crc	= crc16(sendbuf,in);
	sendbuf[in++] = HUINT16(crc);
	sendbuf[in++] = LUINT16(crc);

	yserial_clear(msg->port);
    yserial_write(msg->port,(const char *)sendbuf,in);
	LIFT_LOG(1,sendbuf,in);
	LIFT_clear();
	res = LIFT_recv(msg,2000);
	LIFT_LOG(2,msg->recvbuf,msg->recvlen);
	return res;
}



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



static uint8 LIFT_vendoutSendMsg(uint8 bin,uint8 row,uint8 column)
{
    uint8 res,res1,i;
    for(i = 0;i < 3;i++){
        EV_msleep(500);
        res = LIFT_vmcVendingReq(bin,row,column);
        if(res != 1){
            EV_msleep(50);
            res1 = LIFT_vmcVedingResult(bin);
            if(res1 == LIFT_VENDOUT_VENDING){
                return 1;
            }
        }
        else{
            return 1;
        }
    }

    return res;

}

//返回0:失败，1：成功，2：数据错误 3：无货 4：卡货 5：取货门未开启 6：货物未取走 7：未定义错误	0xff：通信失败
uint8 LIFT_vendoutReq(uint8 bin,uint8 row,uint8 column)
{
    uint8 res,flow,tradeResult;
    uint8 vendSent,isNotTalk,isTrading;
    int32 timeout;

    timeout = 60;flow = 0;
	while(timeout > 0){
        EV_LOGD("LIFT_vendoutReq:timeout=%d\n",timeout);
		res = LIFT_vmcStatusReq(bin);
		if(res == LIFT_STATUS_NORMAL){
			flow = 1;
			break;
		}
		else{
            EV_msleep(1000);
            timeout--;
		}
	}
	if(flow == 0){ //有问题
		return res;
	}

	// 3查询
    isNotTalk = 0;
    vendSent = 0;
    isTrading = 0;
	tradeResult = LIFT_VENDOUT_FAIL;
    timeout = 180;
	while(timeout > 0){
        if(vendSent == 0){
            // 2 出货
            res = LIFT_vendoutSendMsg(bin,row,column);
            if(res != 1){
                return LIFT_VENDOUT_COM_ERR;
            }
            vendSent = 1;
            EV_msleep(500);
        }
        else{
            res = LIFT_vmcVedingResult(bin);//检测出货结果
            if(res > 0 && res != LIFT_VENDOUT_VENDING){ //出货结果
                tradeResult = res;
                if(res == LIFT_VENDOUT_GOODS_NOT_TAKE && isTrading == 0){
                    EV_msleep(2000);
                    timeout -= 2;
                    isNotTalk = 1;
                    vendSent = 0;//re send
                }
                else{
                    return res;
                }
            }
            else{
                if(isNotTalk == 1){
                    isNotTalk = 0;
                    isTrading = 1;
                    timeout += 60;
                }
                EV_msleep(1000);
                timeout--;
            }
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


static uint8 LIFT_send363x(ST_LIFT_MSG *msg)
{
    uint8 i,res,in = 0;
    uint16 crc = 0x0000;
    uint8 *sendbuf;

    in = 0;sendbuf = msg->sendbuf;
    sendbuf[in++] = 0xC7;
    sendbuf[in++] = 0x00;
    sendbuf[in++] = msg->addr;
    sendbuf[in++] = msg->cmd;
    sendbuf[in++] = msg->addr;
    sendbuf[in++] = 0x00;

    for(i = 0;i < msg->datalen;i++){
        sendbuf[in++] = msg->data[i];
    }
    sendbuf[I_LEN] = in;
    crc	= crc16(sendbuf,in);
    sendbuf[in++] = HUINT16(crc);
    sendbuf[in++] = LUINT16(crc);

    yserial_clear(msg->port);
    //EV_msleep(100);
    memset(msg->recvbuf,0,sizeof(msg->recvbuf));
    msg->recvlen = 0;
    LIFT_LOG(1,sendbuf,in);
    yserial_write(msg->port,(const char *)sendbuf,in);
    res = LIFT_recv(msg,2000);
    LIFT_LOG(2,msg->recvbuf,msg->recvlen);
    return res;
}

static int Column363x_send(ST_LIFT_MSG *msg)
{
    int res;
    res = LIFT_send363x(msg);
    if (res != 1) {
#if 1
        msg->cmd = VMC_VENDINGRESULT_REQ;
        msg->data[0] = 1;
        msg->datalen = 1;
        res = LIFT_send363x(msg);
        if (res != 1) {
            return LIFT_VENDOUT_COM_ERR;
        }
        if (msg->recvbuf[3] != GCC_VENDINGRESULT_ACK) {
            return LIFT_VENDOUT_DATAERR;
        }
        if (msg->recvbuf[7] == 0) {
            return LIFT_VENDOUT_SUC;
        }
        else {
            return LIFT_VENDOUT_EMPTY;
        }
#endif
        //return LIFT_VENDOUT_COM_ERR;
    }

    if (msg->recvbuf[3] != GCC_VENDING_ACK) {
        return LIFT_VENDOUT_DATAERR;
    }

    if (msg->recvbuf[7] == 0) {
        return LIFT_VENDOUT_SUC;
    }
    else {
        return LIFT_VENDOUT_EMPTY;
    }
}

int32 Column363x_open(const ST_COL_OPEN_REQ *req,ST_COL_OPEN_RPT *rpt)
{
    int32 ret;
    ST_LIFT_MSG mm;
    ST_LIFT_MSG *msg = &mm;

    if(req == NULL || rpt == NULL){
        EV_LOGE("BT_open:s=%x,r=%x",(unsigned int)req,(unsigned int)rpt);
        return 0;
    }

    if(req->addr <= 0){
        EV_LOGE("Column363x_open:req->addr = %d",req->addr);
        return 0;
    }

    if(req->no <= 0){
        EV_LOGE("Column363x_open:req->no = %d",req->no);
        return 0;
    }

    memset((void *)rpt,0,sizeof(ST_COL_OPEN_RPT));
    rpt->addr = req->addr;
    rpt->fd = req->fd;
    rpt->no = req->no;
    msg->port = rpt->fd;
    msg->addr = 0x01;
    msg->bin = req->addr;
    msg->cmd = VMC_VENDING_REQ;
    msg->data[0] = 1;
    msg->data[1] = req->no;
    msg->datalen = 2;
    ret = Column363x_send(msg);

    if(ret == LIFT_VENDOUT_COM_ERR){
        rpt->is_success = 1;
        rpt->result = LIFT_VENDOUT_FAIL;

    }
    else{
        rpt->is_success = 1;
        rpt->result = ret;

    }
   /* */
    return 1;
}








