#include "Column_api.h"
#include "ev_config.h"
#include "db_serial.h"
#include "timer.h"
#include "yoc_serialport.h"
#include "freak_rs_api.h"

typedef struct _st_mdb_msg_{    
    uint8 data[128];    //数据段
    uint8 head;          //包长度
    uint8 len;     //数据段长度
    uint8 sn;       //sn
    uint8 cmd;      //指令
    uint8 no;       //货道号
	uint8 none;     //返回状态status
	uint8 goc;      //goc
	uint8 chk;      //chk
}COL_MSG;

static COL_MSG colrecvmsg;
static COL_MSG colsendmsg;
static int32 port;
static int32 serialNo;



void COL_LOG(uint8 type,uint8 *data,uint8 len)
{
	uint8 i,buf[512] = {0};
	for(i = 0;i < len;i++){
		sprintf((char *)&buf[i * 3],"%02x ",data[i]);
	}
	if(type == 1){
        EV_LOGD("BT-Send[%d]:%s",len,buf);
	}
	else{
        EV_LOGI("BT-Recv[%d]:%s\n",len,buf);
	}
}

uint8 COL_xorCheck(uint8 *msg,uint8 len)
{
	uint8 i, xor = 0;
    for(i=0;i<len;i++)
    {
        xor ^= *(msg+i);
	}
	return xor;

}

//1收到数据,0没有数据
uint8 COL_recv(COL_MSG *msg)
{
    uint8 in = 0,len = 0,ch,out=0;
    int32 r,timeout;
    uint8 xor;
    uint8 buf[FRS_BUF_SIZE] = {0};
    uint8 no,rlen,i,res;
    if(msg == NULL){
        EV_LOGE("EV_bento_recv:msg=NULL");
        return 0;
    }
    //EV_LOGD("BT_recv:port=%d",port);
    timeout = 25000;
    while(timeout > 0)
	{

        //r = DB_getCh(port,(char *)&ch);
        // EV_LOGD("BT_recv:timeout=%d",timeout);
        //if(r <= 0)//读一个字节数据
        //{
       //     EV_msleep(50);
       //     timeout = (timeout <= 50) ? 0 : timeout - 50;
       //     continue;
       // }

        res = FRS_recv(port,&no,buf,&rlen,100);
        if(res != 1){
            EV_msleep(50);
            timeout = (timeout <= 50) ? 0 : timeout - 50;
            continue;
        }

       // Command ACK:     EE＋08+SN+70+Column code +Status+00＋CHK
       // Mission ACK:     EE＋08+SN+70+Column code +Status+00＋CHK 
        //包头

        for(i = 0;i < rlen;i++){
            if(in == 0)
            {
                if(ch == COL_RECV_HEAD)
                {
                    msg->data[in++] = ch;
                }
                else
                {
                    in = 0;
                }
            }
            //len
            else if(in == 1)
            {
                if(ch > 128)
                {
                    in = 0;
                }
                else
                {
                    msg->data[in++] = ch;
                    len = ch;
                }
            }
            //chk
            else if(in >= (len - 1))
            {
                msg->data[in] = ch;
                //COL_LOG(0,msg->data,len);
                xor = COL_xorCheck(msg->data,(len - 1));
                if(xor == msg->data[in])
                {
                    msg->head=msg->data[out++];
                    msg->len=msg->data[out++];
                    msg->sn=msg->data[out++];
                    msg->cmd=msg->data[out++];
                    msg->no=msg->data[out++];
                    msg->none=msg->data[out++];
                    msg->goc=msg->data[out++];
                    msg->chk=msg->data[out++];
                    COL_LOG(0,msg->data,msg->len);
                    return 1;
                }
                else
                {
                    return 0;
                }

            }
            //data
            else
            {
                msg->data[in++] = ch;
            }

        }
    }
    return 0;

}


void PackSNUpdate()
{
	if(colsendmsg.sn >= 255)
		colsendmsg.sn = 1;
	else
		colsendmsg.sn++;
}


uint8 COL_send()
{
	uint8 out = 0;	
	colsendmsg.head=COL_SEND_HEAD;
	colsendmsg.len=COL_SEND_LEN;
	PackSNUpdate();
	colsendmsg.none=COL_SEND_NONE;
	
	
	
	colsendmsg.data[out++] = colsendmsg.head;
	colsendmsg.data[out++] = colsendmsg.len;
	colsendmsg.data[out++] = colsendmsg.sn;
	colsendmsg.data[out++] = colsendmsg.cmd;
	colsendmsg.data[out++] = colsendmsg.no;
	colsendmsg.data[out++] = colsendmsg.none;
	colsendmsg.data[out++] = colsendmsg.goc;
	colsendmsg.chk = COL_xorCheck(colsendmsg.data,out);
	colsendmsg.data[out++] = colsendmsg.chk;
	//EV_LOGD("EVDrvsend >> %02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x\n",colsendmsg.data[0],colsendmsg.data[1],colsendmsg.data[2],
	//	colsendmsg.data[3],colsendmsg.data[4],colsendmsg.data[5],colsendmsg.data[6],colsendmsg.data[7]);
	yserial_clear(port);

    FRS_send(port,2,colsendmsg.data,out);
    //yserial_write(port,colsendmsg.data,out);
	COL_LOG(1,colsendmsg.data,out);
	return 1;
}

//1有出货动作,0没有出货动作
uint8 Column_sendMsg()
{
    uint8 i,res;

	memset((void *)&colrecvmsg,0,sizeof(colrecvmsg));	
    for(i = 0;i < 3;i++)
	{
        COL_send();		
        res = COL_recv(&colrecvmsg);
		//1收到ack
        if(res == 1)
		{
			memset((void *)&colrecvmsg,0,sizeof(colrecvmsg));
			res = COL_recv(&colrecvmsg);
			//2收到结果
            if(res == 1)
            {
                return 1;
            } 
			else
			{
			    return 0;
			}
        }
    }
    return 0;
}


/*********************************************************************************************************
** Function name	:		Column_open
** Descriptions		:		普通柜开门接口
** input parameters	:       req 请求包结构体指针 rpt 回应包结构体指针
** output parameters:		无
** Returned value	:		0通信失败  1通信成功
*********************************************************************************************************/
int32 Column_open(const ST_COL_OPEN_REQ *req,ST_COL_OPEN_RPT *rpt)
{
    int32 ret;

    if(req == NULL || rpt == NULL){
        EV_LOGE("BT_open:s=%x,r=%x",(unsigned int)req,(unsigned int)rpt);
        return 0;
    }

    if(req->addr <= 0){
        EV_LOGE("BT_open:req->addr = %d",req->addr);
        return 0;
    }

    if(req->no <= 0){
        EV_LOGE("BT_open:req->no = %d",req->no);
        return 0;
    }
	

    memset((void *)rpt,0,sizeof(ST_BT_OPEN_RPT));
    rpt->addr = req->addr;
    rpt->fd = req->fd;
    rpt->no = req->no;
    port = rpt->fd;
    serialNo = req->addr;
    colsendmsg.cmd = COL_SEND_OPEN;
    colsendmsg.no = rpt->no;	
	colsendmsg.goc=req->goc;	
    ret = Column_sendMsg();
	
    if(ret == 1)
	{
       rpt->is_success = 1;
       rpt->result = colrecvmsg.none;
       return 1;
    }
    else
	{
        rpt->is_success = 0;
        rpt->result = 0;
        return 0;
    }
   /* */  
    return 0;
}




