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



/*********************************************************************************************************
** Function name:     	crc16
** Descriptions:	    CRCУ���
** input parameters:    msg��Ҫ���������;len���ݳ���
** output parameters:   ��
** Returned value:      CRC������
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
        EV_LOGD("LF-Send[%d]:%s\n",len,buf);
    }
    else{
        EV_LOGD("LF-Recv[%d]:%s\n",len,buf);
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
	LIFT_LOG(1,msg->recvbuf,msg->recvlen);
	return 0;
}



//����ָ��
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

//0x1F ͨ�Ź��� 0x10 ���� 0x11 ����æ 0x12����
uint8 LIFT_vmcStatusReq(uint8 bin)
{
	uint8 res;
	ST_LIFT_MSG *msg = &liftMsg;
	msg->addr = 0x40;
	msg->bin = bin;
	msg->cmd = VMC_STATUS_REQ;
	msg->datalen = 0;
	
	res =  LIFT_send(msg);
	if(res == 1 && msg->recvbuf[I_MT] == GCC_STATUS_ACK){
		//stLiftTable.Error_OVERALLUINT = msg->recvbuf[5];
		if(msg->recvbuf[5] == 0x01){
			return 0x11; //æ
		}

		if(msg->recvbuf[6] == 0x00){
			return 0x10; //����
		}
		else{
			return 0x12; //����
		}
	}
	else{
		return 0x1F; //ͨ�Ź���
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

//����0:ʧ�ܣ�1���ɹ���2�����ݴ��� 3���޻� 4������ 5��ȡ����δ���� 6������δȡ�� 7��δ�������	0xff��ͨ��ʧ��
// 0 δ���� 1�ɹ� 0x88���ڳ���
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
			return 0x88;
		}
		if(msg->recvbuf[I_DATA + 1] == 0x00){
			return 1;
		}

		temp = msg->recvbuf[I_DATA + 2];
		if(temp == 0x00){
			return 1;
		}
		else if(temp == 0x01){
			return 2;
		}
		else if(temp == 0x02){
			return 3;
		}
		else if(temp == 0x03){
			return 4;
		}
		else if(temp == 0x04){
			return 5;
		}
		else if(temp == 0x05){
			return 6;
		}
		else if(temp == 0x06){
			return 7;
		}
		else if(temp == 0x07){
			return 8;
		}
		else{
			return 9;
		}
		//return 9;
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



//����0:ʧ�ܣ�1���ɹ���2�����ݴ��� 3���޻� 4������ 5��ȡ����δ���� 6������δȡ�� 7��δ�������	0xff��ͨ��ʧ��
uint8 LIFT_vendoutReq(uint8 bin,uint8 row,uint8 column)
{
	uint8 res,flow,err;
	uint32 timeout;
	timeout = 30;flow = 0;
	while(timeout > 0){
		res = LIFT_vmcStatusReq(bin);
		if(res == 0x10){
			flow = 1;
			break;
		}
		else{
			if(res != 0x1F){
				EV_msleep(1000);
				timeout--;
			}			
		}
	}
	
	if(flow == 0){ //������
		return res;
	}
	
	//���г�������
	timeout = 60;
	while(timeout > 0){
		res = LIFT_vmcVendingReq(bin,row,column);
		if(res == 1){
			EV_msleep(500);
			res = LIFT_vmcVendingResultByTime(bin,5000);
			if(res == 6){ //����ûȡ��
				EV_msleep(2000);
				timeout--;
				timeout--;
				err = 6;
				continue;
			}
			else{
				flow = 1;break;
			}
		}
		else{
			res = LIFT_vmcVedingResult(bin);
			if(res == 0x88){
				flow = 1;break;
			}
			EV_msleep(1000);
			timeout--;
		}
	}
	
	if(flow == 0){return err == 6 ? 6 : 0;}
	
	timeout = 50 + row * 3;
	while(timeout > 0){
		res = LIFT_vmcVedingResult(bin);//���������
		if(res > 0 && res != 0x88){ //�������
			return res;
		}
		else{
			if(res > 0){
				EV_msleep(1000);
				timeout--;
			}
		}
	}
	return 0;
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



