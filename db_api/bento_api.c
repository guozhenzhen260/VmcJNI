#include "bento_api.h"
#include "ev_config.h"
#include "db_serial.h"
#include "timer.h"
#include "yoc_serialport.h"

typedef struct _st_mdb_msg_{
    uint8 data[128];    //数据段
    uint8 len;          //包长度
    uint8 data_len;     //数据段长度
    uint8 addr;
    uint8 cmd;
    uint8 arg;
    int32 port;
}BENTO_MSG;

static BENTO_MSG recvmsg;
static BENTO_MSG sendmsg;


static void BT_LOG(uint8 type,uint8 *data,uint8 len)
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


static uint16 BT_crcCheck(uint8 *msg,uint8 len)
{
    uint16 i, j, crc = 0, current = 0;
    for(i=0;i<len;i++) {
        current = msg[i] << 8;
        for(j=0;j<8;j++) {
            if((short)(crc^current)<0)
                crc = (crc<<1)^0x1021;
            else
                crc <<= 1;
            current <<= 1;
        }
    }
    return crc;

}


static uint8 BT_recv(BENTO_MSG *msg)
{
    uint8 in = 0,len = 0,ch;
    int32 r,timeout;
    uint16 crc;
    if(msg == NULL){
        EV_LOGE("EV_bento_recv:msg=NULL");
        return 0;
    }
    //EV_LOGD("BT_recv:port=%d",msg->port);
    timeout = 1500;
    while(timeout){
        r = DB_getCh(msg->port,(char *)&ch);
       // EV_LOGD("BT_recv:timeout=%d",timeout);
        if(r <= 0){
            EV_msleep(50);
            timeout = (timeout <= 50) ? 0 : timeout - 50;
            continue;
        }

        if(in == 0){
            if(ch == (BT_HEAD + 1)){
                msg->data[in++] = ch;
            }
            else{
                in = 0;
            }
        }
        else if(in == 1){
            if(ch > 128){
                in = 0;
            }
            else{
                msg->data[in++] = ch;
                len = ch;
            }
        }
        else if(in >= (len + 1)){
            msg->data[in++] = ch;
            crc = BT_crcCheck(msg->data,len);
            if(crc == INTEG16(msg->data[len],msg->data[len + 1])){
                msg->len = len + 2;
                msg->data_len = len;
                BT_LOG(0,msg->data,msg->len);
                return 1;
            }
            else{
                return 0;
            }

        }
        else{
            msg->data[in++] = ch;
        }

    }
    return 0;

}


static uint8 BT_send(BENTO_MSG *msg)
{
    uint8 out = 0;
    uint16 crc;
    msg->data[out++] = BT_HEAD;
    msg->data[out++] = 0x07;
    msg->data[out++] = msg->addr;
    msg->data[out++] = msg->cmd;
    msg->data[out++] = msg->addr;
    msg->data[out++] = msg->addr;//0x08;
    msg->data[out++] = msg->arg;//0x00;
    crc = BT_crcCheck(msg->data,out);
    msg->data[out++] = HUINT16(crc);
    msg->data[out++] = LUINT16(crc);

    yserial_clear(msg->port);
    yserial_write(msg->port,(const char *)msg->data,out);
    BT_LOG(1,msg->data,out);
    return 1;
}


static uint8 BT_sendMsg(BENTO_MSG *sendmsg,BENTO_MSG *recvmsg)
{
    uint8 i,res;

    recvmsg->port = sendmsg->port;
    for(i = 0;i < 3;i++)
	{
        BT_send(sendmsg);
        res = BT_recv(recvmsg);
        if(res == 1){
            if(sendmsg->cmd == BT_OPEN ||
                  recvmsg->data[3] == BT_OPEN_ACK){
                return 1;
            }

            if(sendmsg->cmd == BT_CHECK ||
                  recvmsg->data[3] == BT_CHECK_ACK){
                return 1;
            }
            if(sendmsg->cmd == BT_LIGHT ||
                  recvmsg->data[3] == BT_LIGHT_ACK){
                return 1;
            }
            if(sendmsg->cmd == BT_HOT ||
                  recvmsg->data[3] == BT_HOT_ACK){
                return 1;
            }

            if(sendmsg->cmd == BT_COOL ||
                  recvmsg->data[3] == BT_COOL_ACK){
                return 1;
            }

            return 2;
        }
    }
    return 0;
}




int32 BT_open(const ST_BT_OPEN_REQ *req,ST_BT_OPEN_RPT *rpt)
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
    sendmsg.port = rpt->fd;
    sendmsg.cmd = BT_OPEN;
    sendmsg.addr = rpt->addr - 1;
    sendmsg.arg = rpt->no;
    ret = BT_sendMsg(&sendmsg,&recvmsg);
    if(ret == 1){
       rpt->is_success = 1;
       rpt->result = 1;
       return 1;
    }
    else{
        rpt->is_success = 0;
        rpt->result = 0;
        return 0;
    }
}




int32 BT_light(const ST_BT_LIGHT_REQ *req,ST_BT_LIGHT_RPT *rpt)
{
    int ret = 0;
    if(req == NULL || rpt == NULL){
        EV_LOGE("BT_light:s=%x,r=%x",(unsigned int)req,(unsigned int)rpt);
        return 0;
    }

    if(req->addr <= 0){
        EV_LOGE("BT_light:req->addr = %d",req->addr);
        return 0;
    }

    rpt->addr = req->addr;
    rpt->fd = req->fd;
    rpt->opt = req->opt;
    sendmsg.port = rpt->fd;
    sendmsg.cmd =  BT_LIGHT;
    sendmsg.addr = rpt->addr - 1;
    sendmsg.arg = rpt->opt;
    ret = BT_sendMsg(&sendmsg,&recvmsg);
    if(ret == 1){
        rpt->res = 1;
        return 1;
    }
    else{
        rpt->res = 0;
        return 0;
    }

}



int32 BT_cool(const ST_BT_COOL_REQ *req,ST_BT_COOL_RPT *rpt)
{
    int ret = 0;
    if(req == NULL || rpt == NULL){
        EV_LOGE("BT_cool:s=%x,r=%x",(unsigned int)req,(unsigned int)rpt);
        return 0;
    }

    if(req->addr <= 0){
        EV_LOGE("BT_cool:req->addr = %d",req->addr);
        return 0;
    }

    rpt->addr = req->addr;
    rpt->fd = req->fd;
    rpt->opt = req->opt;
    sendmsg.port = rpt->fd;
    sendmsg.cmd =  BT_COOL;
    sendmsg.addr = rpt->addr - 1;
    sendmsg.arg = rpt->opt;
    ret = BT_sendMsg(&sendmsg,&recvmsg);
    if(ret == 1){
        rpt->res = 1;
        return 1;
    }
    else{
        rpt->res = 0;
        return 0;
    }

}



int32 BT_hot(const ST_BT_HOT_REQ *req,ST_BT_HOT_RPT *rpt)
{
    int ret = 0;
    if(req == NULL || rpt == NULL){
        EV_LOGE("BT_hot:s=%x,r=%x",(unsigned int)req,(unsigned int)rpt);
        return 0;
    }

    if(req->addr <= 0){
        EV_LOGE("BT_hot:req->addr = %d",req->addr);
        return 0;
    }

    rpt->addr = req->addr;
    rpt->fd = req->fd;
    rpt->opt = req->opt;
    sendmsg.port = rpt->fd;
    sendmsg.cmd =  BT_HOT;
    sendmsg.addr = rpt->addr - 1;
    sendmsg.arg = rpt->opt;
    ret = BT_sendMsg(&sendmsg,&recvmsg);
    if(ret == 1){
        rpt->res = 1;
        return 1;
    }
    else{
        rpt->res = 0;
        return 0;
    }

}



int32  BT_check(const ST_BT_CHECK_REQ *req,ST_BT_CHECK_RPT *rpt)
{
    int32 ret = 0;
    uint8 *buf,i;

    if(req == NULL || rpt == NULL){
        EV_LOGE("BT_check:s=%x,r=%x",(unsigned int)req,(unsigned int)rpt);
        return 0;
    }


    if(req->addr <= 0){
        EV_LOGE("BT_check:req->addr = %d",req->addr);
        return 0;
    }

    rpt->addr = req->addr;
    rpt->fd = req->fd;

    sendmsg.port = rpt->fd;
    sendmsg.cmd =  BT_CHECK;
    sendmsg.addr = rpt->addr - 1;
    sendmsg.arg = 0;
    ret = BT_sendMsg(&sendmsg,&recvmsg);
    if(ret == 1){
        buf = recvmsg.data;
        rpt->res = 1;
        rpt->sum = buf[6];
        rpt->ishot = (buf[8] & 0x01);
        rpt->iscool = ((buf[8] >> 1) & 0x01);
        rpt->islight = ((buf[8] >> 2) & 0x01);

        memset(rpt->id,0,sizeof(rpt->id));
        rpt->id_len = 7;
        rpt->type = 5;
        for(i = 0;i < 7;i++){
            rpt->id[i] = buf[9 + i];
        }
        return 1;

    }
    else{
        rpt->res = 0;
        EV_LOGW("BT_check: addr = %d Fail!!!\n",rpt->addr);
        return 0;
    }

}




