/*==========================================================================
##                   DB_driver
##
##              Copyright(c) 2015-2016 Yoc All rights reserved.
##
##--------------------------------------------------------------------------
## File Description
## ----------------
##      mdb_api.c MDB驱动协议接口c文件
##
##--------------------------------------------------------------------------
##
##  Created by	:	Yoc
##			QQ	:	1760668483
##        Date	:	2015.05.20
##============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "mdb_api.h"
#include "db_serial.h"
#include "timer.h"
#include "ev_config.h"
#include "yoc_serialport.h"

static uint8 sn = 0;

typedef struct _st_mdb_msg_{
    uint8 data[128];    //数据段
    uint8 len;          //包长度
    uint8 data_len; //数据段长度
    uint8 addr;
    uint8 type;
    uint8 subType;
    int32 port;
}MDB_MSG;

static uint8 addr = 0x31;
static MDB_MSG send_msg;
static MDB_MSG recv_msg;

static void MDB_LOG(uint8 type,uint8 *data,uint8 len)
{
    uint8 i,buf[512] = {0};
    for(i = 0;i < len;i++){
        sprintf((char *)&buf[i * 3],"%02x ",data[i]);
    }
    if(type == 1){
        EV_LOGD("MDB-Send[%d]:%s",len,buf);
    }
    else{
        EV_LOGI("MDB-Recv[%d]:%s\n",len,buf);
    }
}



static uint8 MDB_xorCheck(uint8 *msg, uint8 len)
{
    uint8 crc = 0;
    uint8 i;
    for(i = 0;i < len;i++) {
        crc = crc ^ msg[i];
    }
    return crc;
}


uint8 MDB_ackIsRight(MDB_MSG *msg)
{
    if(msg == NULL){return 0;}
    if(msg->type == DB_MT_ACTION_REQ){
        if(msg->data[MT] != DB_MT_ACTION_RPT){
            return 0;
        }

        if(msg->data[MT + 1] == msg->subType){
            return 1;
        }
        else{
            return 0;
        }
    }
    else if(msg->type == DB_MT_CHECK_REQ){
        if(msg->data[MT] != DB_MT_CHECK_RPT){
            return 0;
        }

        if(msg->data[MT + 1] == msg->subType){
            return 1;
        }
        else{
            return 0;
        }

    }
    else if(msg->type == DB_MT_CON_REQ){
        if(msg->data[MT] == DB_MT_CON_RPT){
            return 1;
        }
        else{
            return 0;
        }
    }
    else if(msg->type == DB_MT_ID_REQ){
        if(msg->data[MT] == DB_MT_ID_RPT){
            return 1;
        }
        else{
            return 0;
        }
    }
    else {
        return 0;
    }

}



uint8 MDB_recv(MDB_MSG *msg,uint32 timeout)
{
    uint8 ch,index = 0;
    uint8 crc;
    uint32 t = timeout;
    uint8 buf[FRS_BUF_SIZE] = {0};
    uint8 no,rlen,i;
    int32 res;
    while(t){
        res = FRS_recv(msg->port,&no,buf,&rlen,100);
        if(res != 1){
            EV_msleep(50);
            t = (t <= 50) ? 0 : t - 50;
            continue;
        }

        for(i = 0;i < rlen;i++){
            //EV_LOGD("ch=%x,index=%d",ch,index);
            ch = buf[i];
            if(index == SF){
                if(ch == HEAD_DB){
                    msg->data[index++] = ch;
                }
            }
            else if(index == LEN){
                if(ch > 200 || ch <= MT){
                    index = 0;
                }
                else{
                    msg->data[index++] = ch;
                }
            }
            else if(index >= msg->data[LEN]){
                msg->data[index++] = ch;
                crc = MDB_xorCheck(msg->data,msg->data[LEN]);
                if(crc == ch && sn == msg->data[SN] && MDB_ackIsRight(msg)){
                    msg->len = index;
                    msg->data_len = msg->data[LEN] - MT - 1;
                    MDB_LOG(0,msg->data,msg->len);
                    return 1;
                }
                else{
                    index = 0;
                }
            }
            else{
                msg->data[index++] = ch;
            }
        }


    }
    return 0;
}



void MDB_package(MDB_MSG *msg)
{
    uint8 crc = 0,len;
    len = msg->len;
    msg->data[SF] = HEAD_PC;
    msg->data[LEN] = len;//len
    msg->data[ADDR] = addr;
    msg->data[SN] = ++sn;
    msg->data[MT] = msg->type;

    crc = MDB_xorCheck(msg->data,len);
    msg->data[len] = crc;
    msg->len = len + 1;
}

uint8 MDB_send(MDB_MSG *msg){

    MDB_LOG(1,msg->data,msg->len);

    return FRS_send(msg->port,1,(char *)msg->data,msg->len);

    //return yserial_write(msg->port,(char *)msg->data,msg->len);
}



uint8 MDB_sendMsg(MDB_MSG *send_msg,MDB_MSG *recv_msg,int32 timeout)
{
    uint16 t,i;
    uint8 res;
    t = (timeout <= 2000)  ? 1 : timeout / 2000;
    recv_msg->port = send_msg->port;
    recv_msg->type = send_msg->type;
    recv_msg->subType = send_msg->data[MT + 1];
    for(i = 0;i < t;i++){
        EV_LOGI("MDB_Re_send:i=%d,t=%d,fd=%d",i,t,recv_msg->port);
        MDB_send(send_msg);
        res = MDB_recv(recv_msg,2000);
        if(res == 1){ //收到一次
            return 1;
        }
    }
    return 0;
}

uint8 MDB_pcEncodAmount(uint32 amount)
{

    uint8 i = 0,value;
    if(amount == 0)
        return 0;
    while(!(amount % 10)){
        amount = amount / 10;
        i++;
    }
    switch(amount){
        case 1: 	value = 1;	break;
        case 2: 	value = 2;	break;
        case 5: 	value = 5;	break;
        default: 	value = 0;	break;
    }
    if(value){
        value = (i << 4) | (value & 0x0f);
        return value;
    }
    else
        return 0;

}

uint32 MDB_pcAnalysisAmount(const uint8 data)
{

    uint32 amount;
    uint8 uint;
    if(data == 0)
        return 0;
    uint =  data >> 4;
    amount = data & 0x0f;
    while(uint)
    {
        amount = amount * 10;
        uint--;
    }
    return amount;
}






/*********************************************************************************************************
** Function name:     	MDB_init
** Descriptions:	    现金模组初始化
** input parameters:    无
** output parameters:   无
** Returned value:      0失败  1成功
*********************************************************************************************************/
int32 MDB_init(ST_MDB_INIT_REQ *req,ST_MDB_INIT_RPT *rpt)
{
    uint8 in = MT + 1,res,obj = 0;
    uint8 *buf;
    if(req == NULL || req == NULL){
        EV_LOGE("MDB_init:s=%x,r=%x",(unsigned int)req,(unsigned int)rpt);
        return 0;
    }

    EV_LOGD("MDB_init[%x]:fd=%d,b=%d,c=%d\n",(unsigned int)req,req->fd,req->bill,req->coin);
    memset(rpt,0,sizeof(ST_MDB_INIT_RPT));
    rpt->fd = req->fd;
    rpt->bill = req->bill;
    rpt->coin = req->coin;
    obj = 0;
    if(rpt->bill == 1){obj |= OBJ_BILL;}
    if(rpt->coin == 1){obj |= OBJ_COIN;}

    send_msg.port = req->fd;
    send_msg.type = DB_MT_ACTION_REQ;
    send_msg.data[in++] = MDB_INIT_REQ;
    send_msg.data[in++] = obj;
    send_msg.data[in++] = 0x02;
    send_msg.data[in++] = 0x02;
    send_msg.len = in;
    MDB_package(&send_msg);
    res = MDB_sendMsg(&send_msg,&recv_msg,15000);
    if(res != 1){
        EV_LOGE("MDB_init:Timeout...");
        return 0;
    }

//    if(recv_msg.data_len != 4){ //数据长度不符合
//        EV_LOGE("MDB_init:len=%d != %d",recv_msg.data_len,4);
//        return 0;
//    }

    buf = recv_msg.data;
    if(buf[MT] != DB_MT_ACTION_RPT || buf[MT + 1] != MDB_INIT_RPT){
        EV_LOGE("MDB_init:buf[MT]=%x,buf[MT+1]=%x",buf[MT],buf[MT + 1]);
        return 0;
    }


    if(buf[MT + 3] == 1){
        rpt->billResult = 1;
    }
    else{
        rpt->billResult = 0;
    }

    if(buf[MT + 4] == 1){
        rpt->coinResult = 1;
    }
    else{
        rpt->coinResult = 0;
    }

    rpt->res = 1;
    return 1;

}


/*********************************************************************************************************
** Function name:     	MDB_enable
** Descriptions:	    现金模组使能控制
** input parameters:    enable
** output parameters:   无
** Returned value:      0失败  1成功
*********************************************************************************************************/
int32 MDB_enable(ST_MDB_ENABLE_REQ *req,ST_MDB_ENABLE_RPT *rpt)
{
    uint8 in = MT + 1,res,obj = 0;
    uint8 *buf;
    if(req == NULL || req == NULL){
        EV_LOGE("MDB_enable:req=%x,rpt=%x",(unsigned int)req,(unsigned int)rpt);
        return 0;
    }
    EV_LOGD("MDB_enable[%x]:fd=%d,b=%d,c=%d\n",(unsigned int)req,req->fd,req->bill,req->coin);
    memset(rpt,0,sizeof(ST_MDB_ENABLE_RPT));
    rpt->fd = req->fd;
    rpt->bill = req->bill;
    rpt->coin = req->coin;
    rpt->opt = req->opt;

    obj = 0;
    if(req->bill == 1){obj |= OBJ_BILL;}
    if(req->coin == 1){obj |= OBJ_COIN;}
    send_msg.port = req->fd;
    send_msg.type = DB_MT_ACTION_REQ;
    send_msg.data[in++] = MDB_CONTROL_REQ;
    send_msg.data[in++] = obj;
    send_msg.data[in++] = (req->opt == 1) ?  1 : 2;
    send_msg.len = in;
    MDB_package(&send_msg);
    res = MDB_sendMsg(&send_msg,&recv_msg,5000);

    if(res != 1){
        EV_LOGE("MDB_enable:Timeout...");
        return 0;
    }

//    if(recv_msg.data_len != 4){ //数据长度不符合
//        EV_LOGE("MDB_enable:len=%d != %d",recv_msg.data_len,4);
//        return 0;
//    }

    buf = recv_msg.data;
    if(buf[MT] != DB_MT_ACTION_RPT || buf[MT+1] != MDB_CONTROL_RPT){
        EV_LOGE("MDB_enable:buf[MT]=%x,buf[MT+1]=%x",buf[MT],buf[MT + 1]);
        return 0;
    }


    if(buf[MT + 3] == 1){
        rpt->billResult = 1;
    }
    else{
        rpt->billResult = 0;
    }

    if(buf[MT + 4] == 1){
        rpt->coinResult = 1;
    }
    else{
        rpt->coinResult = 0;
    }
    rpt->res = 1;
    return 1;

}



/*********************************************************************************************************
** Function name:     	MDB_payout
** Descriptions:	    现金模组出币操作
** input parameters:
** output parameters:   无
** Returned value:      0失败  1成功
*********************************************************************************************************/
int32 MDB_payout(const ST_MDB_PAYOUT_REQ *req,ST_MDB_PAYOUT_RPT *rpt)
{
    uint8 in = MT + 1,res,obj = 0;
    uint8 *buf;
    uint32 billAmount = 0,coinAmount = 0;

    if(req == NULL || rpt == NULL){
        EV_LOGE("MDB_payout:req=%x,rpt=%x",(unsigned int)req,(unsigned int)rpt);
        return 0;
    }
    memset(rpt,0,sizeof(ST_MDB_PAYOUT_RPT));
    rpt->fd = req->fd;
    rpt->bill = req->bill;
    rpt->coin = req->coin;
    rpt->billPayout = req->billPayout;
    rpt->coinPayout = req->coinPayout;


    obj = 0;
    if(rpt->bill == 1){
        if(rpt->billPayout > 0){
            obj |= OBJ_BILL;
            billAmount = (uint32)rpt->billPayout;
        }
        else{
            rpt->billChanged = 0;
            billAmount = 0;
        }
    }

    if(rpt->coin == 1){
        if(rpt->coinPayout > 0){
           obj |= OBJ_COIN;
           coinAmount = (uint32)rpt->coinPayout;
        }
        else{
            rpt->coinChanged = 0;
            coinAmount = 0;
        }
    }

    if(billAmount == 0 && coinAmount == 0){

        rpt->res = 1;
        return 1;
    }


    send_msg.port = rpt->fd;
    send_msg.type = DB_MT_ACTION_REQ;
    send_msg.data[in++] = MDB_PAYOUT_REQ;

    send_msg.data[in++] = obj;
    send_msg.data[in++] = H0UINT32(billAmount);
    send_msg.data[in++] = H1UINT32(billAmount);
    send_msg.data[in++] = L0UINT32(billAmount);
    send_msg.data[in++] = L1UINT32(billAmount);

    send_msg.data[in++] = H0UINT32(coinAmount);
    send_msg.data[in++] = H1UINT32(coinAmount);
    send_msg.data[in++] = L0UINT32(coinAmount);
    send_msg.data[in++] = L1UINT32(coinAmount);

    send_msg.len = in;
    MDB_package(&send_msg);
    res = MDB_sendMsg(&send_msg,&recv_msg,600000);

    if(res != 1){
        EV_LOGE("MDB_payout:Timeout...");
        return 0;
    }


    buf = recv_msg.data;
    if(buf[MT] != DB_MT_ACTION_RPT || buf[MT + 1] != MDB_PAYOUT_RPT){
        EV_LOGE("MDB_payout:buf[MT]=%x,buf[MT+1]=%x",buf[MT],buf[MT + 1]);
        return 0;
    }

    rpt->billChanged = INTEG32(buf[MT + 3],buf[MT + 4],buf[MT + 5],buf[MT + 6]);
    rpt->coinChanged = INTEG32(buf[MT + 7],buf[MT + 8],buf[MT + 9],buf[MT + 10]);
    rpt->res = 1;
    return 1;

}

/*********************************************************************************************************
** Function name:     	MDB_hpPayout
** Descriptions:	    hopper模组出币操作
** input parameters:
** output parameters:   无
** Returned value:      0失败  1成功
*********************************************************************************************************/
int32 MDB_hpPayout(const ST_MDB_HP_PAYOUT_REQ *req,ST_MDB_HP_PAYOUT_RPT *rpt)
{
    uint8 in = MT + 1,res;
    uint8 *buf;


    if(req == NULL || rpt == NULL){
        EV_LOGE("MDB_hpPayout:req=%x,rpt=%x",(unsigned int)req,(unsigned int)rpt);
        return 0;
    }

    memset(rpt,0,sizeof(ST_MDB_HP_PAYOUT_RPT));
    rpt->fd = req->fd;
    rpt->no = req->no;
    rpt->nums = req->nums;

    send_msg.port = rpt->fd;
    send_msg.type = DB_MT_ACTION_REQ;
    send_msg.data[in++] = MDB_HP_PAYOUT_REQ;
    send_msg.data[in++] = req->no;
    send_msg.data[in++] = HUINT16(req->nums);
    send_msg.data[in++] = LUINT16(req->nums);
    send_msg.len = in;
    MDB_package(&send_msg);
    res = MDB_sendMsg(&send_msg,&recv_msg,600000);

    if(res != 1){
        EV_LOGE("MDB_hpPayout:Timeout...");
        return 0;
    }


    buf = recv_msg.data;
    if(buf[MT] != DB_MT_ACTION_RPT || buf[MT + 1] != MDB_HP_PAYOUT_RPT){
        EV_LOGE("MDB_hpPayout:buf[MT]=%x,buf[MT+1]=%x",buf[MT],buf[MT + 1]);
        return 0;
    }

    rpt->changed = INTEG16(buf[MT + 3],buf[MT + 4]);
    rpt->res = 1;
    return 1;

}



/*********************************************************************************************************
** Function name:     	MDB_payback
** Descriptions:	    现金模组退币指示
** input parameters:
** output parameters:   无
** Returned value:      0失败 1成功
*********************************************************************************************************/
int32 MDB_payback(const ST_MDB_PAYBACK_REQ *req,ST_MDB_PAYBACK_RPT *rpt)
{
    uint8 in = MT + 1,res,obj;
    uint8 *buf;
    uint32 amount;

    if(req == NULL || req == NULL){
        EV_LOGE("MDB_payback:s=%x,r=%x",(unsigned int)req,(unsigned int)rpt);
        return 0;
    }

    memset(rpt,0,sizeof(ST_MDB_PAYBACK_RPT));
    rpt->fd = req->fd;
    rpt->bill = req->bill;
    rpt->coin = req->coin;

    obj = 0;
    if(rpt->bill == 1){
        obj |= OBJ_BILL;
    }
    if(rpt->coin == 1){
        obj |= OBJ_COIN;
    }
    send_msg.port = rpt->fd;
    send_msg.type = DB_MT_ACTION_REQ;
    send_msg.data[in++] = MDB_PAYBACK_REQ;
    send_msg.data[in++] = obj;
    send_msg.len = in;
    MDB_package(&send_msg);
    res = MDB_sendMsg(&send_msg,&recv_msg,600000); //10分钟超时

    if(res != 1){
        EV_LOGE("MDB_payback:Timeout...");
        return 0;
    }


    buf = recv_msg.data;
    if(buf[MT] != DB_MT_ACTION_RPT || buf[MT + 1] != MDB_PAYBACK_RPT){
        EV_LOGE("MDB_payback:buf[MT]=%x,buf[MT+1]=%x",buf[MT],buf[MT + 1]);
        return 0;
    }

    amount = INTEG32(buf[MT + 3],buf[MT + 4],buf[MT + 5],buf[MT + 6]);
    rpt->billChanged = (int32)amount;
    amount = INTEG32(buf[MT + 7],buf[MT + 8],buf[MT + 9],buf[MT + 10]);
    rpt->coinChanged = (int32)amount;
    rpt->res = 1;
    return 1;


}


/*********************************************************************************************************
** Function name:     	MDB_cost
** Descriptions:	    现金模组扣款指示
** input parameters:    fd 串口编号 cost扣款金额 timeout 超时时间
** output parameters:   无
** Returned value:      0失败 1成功
*********************************************************************************************************/
int32 MDB_cost(const ST_MDB_COST_REQ *req,ST_MDB_COST_RPT *rpt)
{
    uint8 in = MT + 1,res;
    uint32 amount;
    uint8 *buf;
    if(req == NULL || req == NULL){
        EV_LOGE("MDB_cost:s=%x,r=%x",(unsigned int)req,(unsigned int)rpt);
        return 0;
    }

    memset(rpt,0,sizeof(ST_MDB_COST_RPT));
    rpt->fd = req->fd;
    rpt->cost = req->cost;

    if(rpt->cost <= 0){
        EV_LOGW("MDB_cost:cost = %d",rpt->cost);
        return 0;
    }


    amount = (uint32)rpt->cost;
    send_msg.port = rpt->fd;
    send_msg.type = DB_MT_ACTION_REQ;
    send_msg.data[in++] = MDB_COST_REQ;
    send_msg.data[in++] = 0x03;
    send_msg.data[in++] = H0UINT32(amount);
    send_msg.data[in++] = H1UINT32(amount);
    send_msg.data[in++] = L0UINT32(amount);
    send_msg.data[in++] = L1UINT32(amount);

    send_msg.len = in;
    MDB_package(&send_msg);
    res = MDB_sendMsg(&send_msg,&recv_msg,5000);

    if(res != 1){
        EV_LOGE("MDB_cost:Timeout...");
        return 0;
    }

    buf = recv_msg.data;
    if(buf[MT] != DB_MT_ACTION_RPT || buf[MT + 1] != MDB_COST_RPT){
        EV_LOGE("MDB_cost:buf[MT]=%x,buf[MT+1]=%x",buf[MT],buf[MT + 1]);
        return 0;
    }
    amount = INTEG32(buf[MT + 3],buf[MT + 4],buf[MT + 5],buf[MT + 6]);
    rpt->billAmount = (int32)amount;
    amount = INTEG32(buf[MT + 7],buf[MT + 8],buf[MT + 9],buf[MT + 10]);
    rpt->coinAmount = (int32)amount;
    rpt->res = 1;
    return 1;

}


/*********************************************************************************************************
** Function name	:		MDB_bill_config
** Descriptions		:		MDB设备纸币器配置接口
** input parameters	:       req 请求包结构体指针 rpt 回应包结构体指针
** output parameters:		无
** Returned value	:		0通信失败  1通信成功
*********************************************************************************************************/
int32 MDB_bill_config(const ST_MDB_BILL_CON_REQ *req,ST_MDB_BILL_CON_RPT *rpt)
{
    uint8 in = MT + 1,res,i;
    uint8 type;
    uint8 *buf;
    if(req == NULL || req == NULL){
        EV_LOGE("MDB_bill_config:s=%x,r=%x",(unsigned int)req,(unsigned int)rpt);
        return 0;
    }

    memset(rpt,0,sizeof(ST_MDB_COST_RPT));
    rpt->fd = req->fd;
    rpt->acceptor = req->acceptor;
    rpt->dispenser = req->dispenser;
    for(i = 0;i < 16;i++){
        rpt->ch_d[i] = req->ch_d[i];
        rpt->ch_r[i] = req->ch_r[i];
    }


    type = (uint8)(rpt->acceptor & 0x0F);
    type |= ((uint8)(rpt->dispenser & 0x0F) << 4);
    send_msg.port = rpt->fd;
    send_msg.type = DB_MT_ACTION_REQ;
    send_msg.data[in++] = MDB_BILL_CON_REQ;
    send_msg.data[in++] = type;
    for(i = 0;i < 16;i++){
        send_msg.data[in++] = MDB_pcEncodAmount(rpt->ch_r[i]);
    }
    for(i = 0;i < 16;i++){
        send_msg.data[in++] = MDB_pcEncodAmount(rpt->ch_d[i]);
    }

    send_msg.len = in;
    MDB_package(&send_msg);
    res = MDB_sendMsg(&send_msg,&recv_msg,5000);

    if(res != 1){
        EV_LOGE("MDB_bill_config:Timeout...");
        return 0;
    }

    buf = recv_msg.data;
    if(buf[MT] == DB_MT_ACTION_RPT && buf[MT + 1] == MDB_BILL_CON_RPT){
        rpt->com_ok = 1;
        rpt->res = buf[MT + 2];
        return 1;

    }
    else{
        return 0;
    }

}

/*********************************************************************************************************
** Function name	:		EV_mdbCoinConfig
** Descriptions		:		MDB设备硬币器配置接口
** input parameters	:       req 请求包结构体指针 rpt 回应包结构体指针
** output parameters:		无
** Returned value	:		0通信失败  1通信成功
*********************************************************************************************************/
int32 MDB_coin_config(const ST_MDB_COIN_CON_REQ *req,ST_MDB_COIN_CON_RPT *rpt)
{
    uint8 in = MT + 1,res,i;
    uint8 type;
    uint8 *buf;

    if(req == NULL || req == NULL){
        EV_LOGE("MDB_coin_config:s=%x,r=%x",(unsigned int)req,(unsigned int)rpt);
        return 0;
    }

    memset(rpt,0,sizeof(ST_MDB_COST_RPT));
    rpt->fd = req->fd;
    rpt->acceptor = req->acceptor;
    rpt->dispenser = req->dispenser;
    rpt->high_enable = req->high_enable;
    EV_LOGD("MDB_coin_config:\n\n");
    for(i = 0;i < 16;i++){
        rpt->ch_d[i] = req->ch_d[i];
        rpt->ch_r[i] = req->ch_r[i];
        EV_LOGD("ch_d[%d] = %d ch_r[%d] = %d\n",i,rpt->ch_d[i],i,rpt->ch_r[i]);
    }

    type = (uint8)(rpt->acceptor & 0x0F);
    type |= ((uint8)(rpt->dispenser & 0x0F) << 4);
    send_msg.port = rpt->fd;
    send_msg.type = DB_MT_ACTION_REQ;
    send_msg.data[in++] = MDB_COIN_CON_REQ;
    send_msg.data[in++] = type;
    send_msg.data[in++] = rpt->high_enable;
    for(i = 0;i < 16;i++){
        send_msg.data[in++] = MDB_pcEncodAmount(rpt->ch_r[i]);
    }
    for(i = 0;i < 16;i++){
        send_msg.data[in++] = MDB_pcEncodAmount(rpt->ch_d[i]);
    }

    send_msg.len = in;
    MDB_package(&send_msg);
    res = MDB_sendMsg(&send_msg,&recv_msg,5000);
    if(res != 1){
        EV_LOGE("MDB_coin_config:Timeout...");
        return 0;
    }

    buf = recv_msg.data;
    if(buf[MT] == DB_MT_ACTION_RPT && buf[MT + 1] == MDB_COIN_CON_RPT){
        rpt->com_ok = 1;
        rpt->res = buf[MT + 2];
        return 1;

    }
    else{
        return 0;
    }

}



/*********************************************************************************************************
** Function name:     	MDB_heart_check
** Descriptions:	    现金模组心跳查询
** input parameters:    con
** output parameters:   无
** Returned value:      0失败 1成功
*********************************************************************************************************/
int32 MDB_heart_check(ST_MDB_HEART_REQ *req,ST_MDB_HEART_RPT *rpt)
{
    uint8 in = MT + 1,res,out = MT + 2;
    uint8 temp,i;
    uint16 temp16;
    uint32 temp32;
    uint8 *buf;
    if(req == NULL || req == NULL){
        EV_LOGE("MDB_heart_check:s=%x,r=%x",(unsigned int)req,(unsigned int)rpt);
        return 0;
    }

    memset(rpt,0,sizeof(ST_MDB_HEART_RPT));
    rpt->fd = req->fd;

    send_msg.port = req->fd;
    send_msg.type = DB_MT_CHECK_REQ;
    send_msg.data[in++] = MDB_CHECK_REQ;
    send_msg.len = in;
    MDB_package(&send_msg);
    res = MDB_sendMsg(&send_msg,&recv_msg,5000);

    if(res != 1){
        EV_LOGE("MDB_heart_check:Timeout...");
        return 0;
    }

//    if(recv_msg.data_len != 23){ //数据长度不符合
//        EV_LOGE("MDB_heart_check:len=%d != %d",recv_msg.data_len,23);
//        return 0;
//    }

    buf = recv_msg.data;
    if(buf[MT] != DB_MT_CHECK_RPT || buf[MT + 1] != MDB_CHECK_RPT){
        EV_LOGE("MDB_heart_check:buf[MT]=%x,buf[MT+1]=%x",buf[MT],buf[MT + 1]);
        return 0;
    }

    //纸币器
    temp = buf[out++];
    if(temp & BILL_BIT_DISABLE){
        rpt->billEnable = 0;
    }
    else{
        rpt->billEnable = 1;
    }
    if(temp & BILL_BIT_PAYBACK){
        rpt->billPayback = 1;
    }
    else{
        rpt->billPayback = 0;
    }

    temp16 = INTEG16(buf[out + 0],buf[out + 1]);
    out += 2;

    //解析 故障码
    if(temp16 & BILL_ERR_COM){
        rpt->billErr = 1;
    }
    else if(temp16 & BILL_ERR_SENSOR){
         rpt->billErr = 2;
    }
    else if(temp16 & BILL_ERR_TUBEJAM){
         rpt->billErr = 3;
    }
    else if(temp16 & BILL_ERR_ROM){
         rpt->billErr = 4;
    }
    else if(temp16 & BILL_ERR_ROUTING){
         rpt->billErr = 5;
    }
    else if(temp16 & BILL_ERR_JAM){
         rpt->billErr = 6;
    }
    else if(temp16 & BILL_ERR_REMOVECASH){
         rpt->billErr = 7;
    }
    else if(temp16 & BILL_ERR_DISABLE){
         rpt->billErr = 8;
    }
    else if(temp16 & BILL_ERR_MOTO){
         rpt->billErr = 9;
    }
    else if(temp16 & BILL_ERR_CASH){
         rpt->billErr = 10;
    }
    else if(temp16 & BILL_ERR_UNKNOW){
         rpt->billErr = 99;
    }
    else{
         rpt->billErr = 0;
    }




    temp32 = INTEG32(buf[out],buf[out + 1],buf[out + 2],buf[out + 3]);
    out += 4;
    rpt->billAmount = (int32)temp32;

    temp32 = INTEG32(buf[out],buf[out + 1],buf[out + 2],buf[out + 3]);
    out += 4;
    rpt->billRemain = (int32)temp32;


    //硬币器
    temp = buf[out++];
    if(temp & COIN_BIT_DISABLE){
        rpt->coinEnable = 0;
    }
    else{
        rpt->coinEnable = 1;
    }

    if(temp & COIN_BIT_PAYBACK){
        rpt->coinPayback = 1;
    }
    else{
        rpt->coinPayback = 0;
    }

    temp16 = INTEG16(buf[out + 0],buf[out + 1]);
    out += 2;

    if(temp & COIN_BIT_HOPPER){
        rpt->coinErr = 0;
        for(i = 0;i < 8;i++){
            rpt->hopper[i] = (temp16 >> (i * 2)) & 0x03;
        }
        rpt->isHopper = 1;
    }
    else{
        rpt->isHopper = 0;
        if(temp16 & COIN_ERR_COM){
            rpt->coinErr = 1;
        }
        else if(temp16 & COIN_ERR_SENSOR){
            rpt->coinErr = 2;
        }
        else if(temp16 & COIN_ERR_TUBEJAM){
            rpt->coinErr = 3;
        }
        else if(temp16 & COIN_ERR_ROM){
            rpt->coinErr = 4;
        }
        else if(temp16 & COIN_ERR_ROUTING){
            rpt->coinErr = 5;
        }
        else if(temp16 & COIN_ERR_JAM){
            rpt->coinErr = 6;
        }
        else if(temp16 & COIN_ERR_REMOVETUBE){
            rpt->coinErr = 7;
        }
        else if(temp16 & COIN_ERR_UNKNOW){
            rpt->coinErr = 99;
        }
        else{
            rpt->coinErr = 0;
        }
    }





    temp32 = INTEG32(buf[out],buf[out + 1],buf[out + 2],buf[out + 3]);
    out += 4;
    rpt->coinAmount = (int32)temp32;

    temp32 = INTEG32(buf[out],buf[out + 1],buf[out + 2],buf[out + 3]);
    out += 4;
    rpt->coinRemain = (int32)temp32;

    rpt->res = 1;
    return 1;


}



/*********************************************************************************************************
** Function name:     	MDB_bill_info_check
** Descriptions:	    纸币器信息查询
** input parameters:    info
** output parameters:   无
** Returned value:      0失败 1成功
*********************************************************************************************************/
int32 MDB_bill_info_check(const ST_MDB_BILL_INFO_REQ *req,ST_MDB_BILL_INFO_RPT *rpt)
{
    uint8 in = MT + 1,res,i,out = MT + 2;
    uint8 temp;
    uint16 temp16;
    uint8 *buf;
    if(req == NULL || req == NULL){
        EV_LOGE("MDB_bill_info_check:s=%x,r=%x",(unsigned int)req,(unsigned int)rpt);
        return 0;
    }

    memset(rpt,0,sizeof(ST_MDB_BILL_INFO_RPT));
    rpt->fd = req->fd;


    send_msg.port = req->fd;
    send_msg.type = DB_MT_CHECK_REQ;
    send_msg.data[in++] = MDB_BILL_INFO_REQ;
    send_msg.len = in;
    MDB_package(&send_msg);
    res = MDB_sendMsg(&send_msg,&recv_msg,5000);

    if(res != 1){
        EV_LOGE("MDB_bill_info_check:Timeout...");
        return 0;
    }


    buf = recv_msg.data;
    if(buf[MT] != DB_MT_CHECK_RPT || buf[MT + 1] != MDB_BILL_INFO_RPT){
        EV_LOGE("MDB_bill_info_check:buf[MT]=%x,buf[MT+1]=%x",buf[MT],buf[MT + 1]);
        return 0;
    }


    //纸币器
    temp = buf[out++];
    rpt->acceptor = (int32)(temp & 0x0F);
    rpt->dispenser = (int32)((temp >> 4) & 0x0F);

    for(i = 0;i < 3;i++){
        rpt->code[i] = buf[out++];
    }
    for(i = 0;i < 12;i++){
        rpt->sn[i] = buf[out++];

    }

    for(i = 0;i < 12;i++){
        rpt->model[i] = buf[out++];
    }

    for(i = 0;i < 2;i++){
        rpt->ver[i] = buf[out++];
    }

    temp16 = INTEG16(buf[out + 0],buf[out + 1]);
    out += 2;
    rpt->capacity = (int32)temp16;

    for(i = 0;i < 16;i++){
        temp = buf[out++];
        rpt->ch_r[i] = (int32)MDB_pcAnalysisAmount(temp);
    }
    for(i = 0;i < 16;i++){
        temp = buf[out++];
        rpt->ch_d[i] = (int32)MDB_pcAnalysisAmount(temp);
    }
    rpt->res = 1;
    return 1;



}

/*********************************************************************************************************
** Function name:     	MDB_coin_info_check
** Descriptions:	    纸币器信息查询
** input parameters:    info
** output parameters:   无
** Returned value:      0失败 1成功
*********************************************************************************************************/
int32 MDB_coin_info_check(const ST_MDB_COIN_INFO_REQ *req,ST_MDB_COIN_INFO_RPT *rpt)
{
    uint8 in = MT + 1,res,i,out = MT + 2;
    uint8 temp;
    uint16 temp16;
    uint8 *buf;
    if(req == NULL || req == NULL){
        EV_LOGE("MDB_coin_info_check:s=%x,r=%x",(unsigned int)req,(unsigned int)rpt);
        return 0;
    }

    memset(rpt,0,sizeof(ST_MDB_COIN_INFO_RPT));
    rpt->fd = req->fd;

    send_msg.port = req->fd;
    send_msg.type = DB_MT_CHECK_REQ;
    send_msg.data[in++] = MDB_COIN_INFO_REQ;
    send_msg.len = in;
    MDB_package(&send_msg);
    res = MDB_sendMsg(&send_msg,&recv_msg,5000);

    if(res != 1){
        EV_LOGE("MDB_coin_info_check:Timeout...");
        return 0;
    }


    buf = recv_msg.data;
    if(buf[MT] != DB_MT_CHECK_RPT || buf[MT + 1] != MDB_COIN_INFO_RPT){
        EV_LOGE("MDB_coin_info_check:buf[MT]=%x,buf[MT+1]=%x",buf[MT],buf[MT + 1]);
        return 0;
    }

    //纸币器
    temp = buf[out++];
    rpt->acceptor = (int32)(temp & 0x0F);
    rpt->dispenser = (int32)((temp >> 4) & 0x0F);

    for(i = 0;i < 3;i++){
        rpt->code[i] = buf[out++];
    }
    for(i = 0;i < 12;i++){
        rpt->sn[i] = buf[out++];

    }

    for(i = 0;i < 12;i++){
        rpt->model[i] = buf[out++];
    }

    for(i = 0;i < 2;i++){
        rpt->ver[i] = buf[out++];
    }



    temp16 = INTEG16(buf[out + 0],buf[out + 1]);
    out += 2;
    rpt->capacity = (int32)temp16;

    for(i = 0;i < 16;i++){
        temp = buf[out++];
        rpt->ch_r[i] = (int32)MDB_pcAnalysisAmount(temp);
    }
    for(i = 0;i < 16;i++){
        temp = buf[out++];
        rpt->ch_d[i] = (int32)MDB_pcAnalysisAmount(temp);
    }
    rpt->res = 1;
    return 1;



}






