/*==========================================================================
##                   DB_driver
##
##              Copyright(c) 2015-2016 Yoc All rights reserved.
##
##--------------------------------------------------------------------------
## File Description
## ----------------
##      DB_driver.c 货道与MDB驱动板库c文件
##
##--------------------------------------------------------------------------
##
##  Created by	:	Yoc
##			QQ	:	1760668483
##        Date	:	2015.05.20
##============================================================================*/
#include "db_driver.h"
#include "yoc_serialport.h"
#include "ev_config.h"
#include "mdb_api.h"
#include "bento_api.h"
#include "Column_api.h"
#include "lift_api.h"


int32   EV_API  EV_portRegister(int8 *portName)
{
    int32 fd;
    fd = yserial_open(portName);
    if(fd == -1){
        EV_LOGE("Port[%s]:open fail!\n",portName);
        return -1;
    }
    yserial_setParity(fd,PAR_NONE);
    yserial_setBaudRate(fd,115200);
    yserial_setDataBits(fd,DATA_8);
    yserial_setStopBits(fd,STOP_1);
    yserial_setTimeout(fd,10);
    yserial_clear(fd);

    EV_LOGI("Port[%s]:open suc!\n",yserial_getPortName(fd));
    return fd;
}






void   EV_API  EV_portRelease(int32 fd)
{
    yserial_close(fd);
    EV_LOGI("Port_no[%d]:released!\n",fd);
}


void   EV_API  EV_portReleaseByName(char *portName)
{
	if(portName == NULL){
		EV_LOGW("Port:is null!\n");
		return;
	}

    yserial_closeByName(portName);
    EV_LOGI("Port_no[%s]:released!\n",portName);
}






int32  EV_API  EV_mdbInit(ST_MDB_INIT_REQ *req,ST_MDB_INIT_RPT *rpt)
{
    int32 res;
    EV_LOGD("EV_mdbInit=%x\n",(unsigned int)req);
    res = MDB_init(req,rpt);
    EV_LOGD("EV:init return res=%d",res);
    return res;
}



/*********************************************************************************************************
** Function name	:		EV_mdbEnable
** Descriptions		:		MDB设备使能接口
** input parameters	:       req 请求包结构体指针 rpt 回应包结构体指针
** output parameters:		无
** Returned value	:		0通信失败  1通信成功
*********************************************************************************************************/
int32  EV_API  EV_mdbEnable(ST_MDB_ENABLE_REQ *req,ST_MDB_ENABLE_RPT *rpt)
{
    int32 res;
    EV_LOGD("EV_mdbEnable req=%x\n",(unsigned int)req);
    res = MDB_enable(req,rpt);
    EV_LOGD("EV_mdbEnable:res=%d",res);
    return res;
}



/*********************************************************************************************************
** Function name	:		EV_mdbHeartCheck
** Descriptions		:		MDB现金设备心跳查询接口 主要上报收币金额 所以需要每个500ms发送一次
** input parameters	:       req 请求包结构体指针 rpt 回应包结构体指针
** output parameters:		无
** Returned value	:		0通信失败  1通信成功
*********************************************************************************************************/
int32  EV_API  EV_mdbHeartCheck(ST_MDB_HEART_REQ *req,ST_MDB_HEART_RPT *rpt)
{
    int32 res;
    EV_LOGD("EV_mdbHeartCheck req=%x\n",(unsigned int)req);
    res = MDB_heart_check(req,rpt);
    EV_LOGD("EV_mdbHeartCheck:res=%d",res);
    return res;
}



/*********************************************************************************************************
** Function name	:		EV_mdbBillInfoCheck
** Descriptions		:		MDB纸币器查询接口
** input parameters	:       req 请求包结构体指针 rpt 回应包结构体指针
** output parameters:		无
** Returned value	:		0通信失败  1通信成功
*********************************************************************************************************/
int32  EV_API  EV_mdbBillInfoCheck(ST_MDB_BILL_INFO_REQ *req,ST_MDB_BILL_INFO_RPT *rpt)
{
    int32 res;
    EV_LOGD("EV_mdbBillInfoCheck req=%x\n",(unsigned int)req);
    res = MDB_bill_info_check(req,rpt);
    EV_LOGD("EV_mdbBillInfoCheck:res=%d",res);
    return res;
}



/*********************************************************************************************************
** Function name	:		EV_mdbCoinInfoCheck
** Descriptions		:		MDB硬币器查询接口
** input parameters	:       req 请求包结构体指针 rpt 回应包结构体指针
** output parameters:		无
** Returned value	:		0通信失败  1通信成功
*********************************************************************************************************/
int32  EV_API  EV_mdbCoinInfoCheck(ST_MDB_COIN_INFO_REQ *req,ST_MDB_COIN_INFO_RPT *rpt)
{
    int32 res;
    EV_LOGD("EV_mdbCoinInfoCheck req=%x\n",(unsigned int)req);
    res = MDB_coin_info_check(req,rpt);
    EV_LOGD("EV_mdbCoinInfoCheck:res=%d",res);
    return res;
}




/*********************************************************************************************************
** Function name	:		EV_mdbCost
** Descriptions		:		MDB设备扣款接口
** input parameters	:       req 请求包结构体指针 rpt 回应包结构体指针
** output parameters:		无
** Returned value	:		0通信失败  1通信成功
*********************************************************************************************************/
int32  EV_API  EV_mdbCost(ST_MDB_COST_REQ *req,ST_MDB_COST_RPT *rpt)
{
    int32 res;
    EV_LOGD("EV_mdbCost req=%x\n",(unsigned int)req);
    res = MDB_cost(req,rpt);
    EV_LOGD("EV_mdbCost:res=%d",res);
    return res;
}



/*********************************************************************************************************
** Function name	:		EV_mdbPayBack
** Descriptions		:		MDB设备退币接口
** input parameters	:       req 请求包结构体指针 rpt 回应包结构体指针
** output parameters:		无
** Returned value	:		0通信失败  1通信成功
*********************************************************************************************************/
int32  EV_API  EV_mdbPayBack(ST_MDB_PAYBACK_REQ *req,ST_MDB_PAYBACK_RPT *rpt)
{
    int32 res;
    EV_LOGD("EV_mdbPayBack:req=%x\n",(unsigned int)req);
    res = MDB_payback(req,rpt);
    EV_LOGD("EV_mdbPayBack:res=%d",res);
    return res;
}




/*********************************************************************************************************
** Function name	:		EV_mdbPayout
** Descriptions		:		MDB设备找币接口
** input parameters	:       req 请求包结构体指针 rpt 回应包结构体指针
** output parameters:		无
** Returned value	:		0通信失败  1通信成功
*********************************************************************************************************/
int32  EV_API  EV_mdbPayout(ST_MDB_PAYOUT_REQ *req,ST_MDB_PAYOUT_RPT *rpt)
{
    int32 res;
    EV_LOGD("EV_mdbPayout:req=%x\n",(unsigned int)req);
    res = MDB_payout(req,rpt);
    EV_LOGD("EV_mdbPayout:res=%d",res);
    return res;
}



/*********************************************************************************************************
** Function name	:		EV_mdbHpPayout
** Descriptions		:		hopper设备找币接口
** input parameters	:       req 请求包结构体指针 rpt 回应包结构体指针
** output parameters:		无
** Returned value	:		0通信失败  1通信成功
*********************************************************************************************************/
int32  EV_API  EV_mdbHpPayout(ST_MDB_HP_PAYOUT_REQ *req,ST_MDB_HP_PAYOUT_RPT *rpt)
{
    int32 res;
    EV_LOGD("EV_mdbHpPayout:req=%x\n",(unsigned int)req);
    res = MDB_hpPayout(req,rpt);
    EV_LOGD("EV_mdbHpPayout:res=%d",res);
    return res;
}




/*********************************************************************************************************
** Function name	:		EV_mdbBillConfig
** Descriptions		:		MDB设备纸币器配置接口
** input parameters	:       req 请求包结构体指针 rpt 回应包结构体指针
** output parameters:		无
** Returned value	:		0通信失败  1通信成功
*********************************************************************************************************/
int32  EV_API  EV_mdbBillConfig(ST_MDB_BILL_CON_REQ *req,ST_MDB_BILL_CON_RPT *rpt)
{
    int32 res;
    EV_LOGD("EV_mdbBillConfig:req=%x\n",(unsigned int)req);
    res = MDB_bill_config(req,rpt);
    EV_LOGD("EV_mdbBillConfig:res=%d",res);
    return res;
}


/*********************************************************************************************************
** Function name	:		EV_mdbCoinConfig
** Descriptions		:		MDB设备硬币器配置接口
** input parameters	:       req 请求包结构体指针 rpt 回应包结构体指针
** output parameters:		无
** Returned value	:		0通信失败  1通信成功
*********************************************************************************************************/
int32  EV_API  EV_mdbCoinConfig(ST_MDB_COIN_CON_REQ *req,ST_MDB_COIN_CON_RPT *rpt)
{
    int32 res;
    EV_LOGD("EV_mdbCoinConfig:req=%x\n",(unsigned int)req);
    res = MDB_coin_config(req,rpt);
    EV_LOGD("EV_mdbCoinConfig:res=%d",res);
    return res;
}





/*********************************************************************************************************
** Function name	:		EV_bentoOpen
** Descriptions		:		BENTO快递柜开门接口
** input parameters	:       req 请求包结构体指针 rpt 回应包结构体指针
** output parameters:		无
** Returned value	:		0通信失败  1通信成功
*********************************************************************************************************/
int32  EV_API  EV_bentoOpen(ST_BT_OPEN_REQ *req,ST_BT_OPEN_RPT *rpt)
{
    int32 res;
    EV_LOGD("EV_bentoOpen:req=%x\n",(unsigned int)req);
    res = BT_open(req,rpt);
    EV_LOGD("EV_bentoOpen:res=%d",res);
    return res;
}



/*********************************************************************************************************
** Function name	:		EV_bentoCheck
** Descriptions		:		BENTO快递柜查询接口
** input parameters	:       req 请求包结构体指针 rpt 回应包结构体指针
** output parameters:		无
** Returned value	:		0通信失败  1通信成功
*********************************************************************************************************/
int32  EV_API  EV_bentoCheck(ST_BT_CHECK_REQ *req,ST_BT_CHECK_RPT *rpt)
{
    int32 res;
    EV_LOGD("EV_bentoCheck:req=%x\n",(unsigned int)req);
    res = BT_check(req,rpt);
    EV_LOGD("EV_bentoCheck:res=%d",res);
    return res;
}




/*********************************************************************************************************
** Function name	:		EV_bentoLight
** Descriptions		:		BENTO快递柜照明控制接口
** input parameters	:       req 请求包结构体指针 rpt 回应包结构体指针
** output parameters:		无
** Returned value	:		0通信失败  1通信成功
*********************************************************************************************************/
int32  EV_API  EV_bentoLight(ST_BT_LIGHT_REQ *req,ST_BT_LIGHT_RPT *rpt)
{
    int32 res;
    EV_LOGD("EV_bentoLight:req=%x\n",(unsigned int)req);
    res = BT_light(req,rpt);
    EV_LOGD("EV_bentoLight:res=%d",res);
    return res;
}


/*********************************************************************************************************
** Function name	:		EV_bentoCool
** Descriptions		:		BENTO快递柜制冷控制接口
** input parameters	:       req 请求包结构体指针 rpt 回应包结构体指针
** output parameters:		无
** Returned value	:		0通信失败  1通信成功
*********************************************************************************************************/
int32  EV_API  EV_bentoCool(ST_BT_COOL_REQ *req,ST_BT_COOL_RPT *rpt)
{
    int32 res;
    EV_LOGD("EV_bentoCool:req=%x\n",(unsigned int)req);
    res = BT_cool(req,rpt);
    EV_LOGD("EV_bentoCool:res=%d",res);
    return res;
}



/*********************************************************************************************************
** Function name	:		EV_bentoHot
** Descriptions		:		BENTO快递柜加热控制接口
** input parameters	:       req 请求包结构体指针 rpt 回应包结构体指针
** output parameters:		无
** Returned value	:		0通信失败  1通信成功
*********************************************************************************************************/
int32  EV_API  EV_bentoHot(ST_BT_HOT_REQ *req,ST_BT_HOT_RPT *rpt)
{
    int32 res;
    EV_LOGD("EV_bentoHot:req=%x\n",(unsigned int)req);
    res = BT_hot(req,rpt);
    EV_LOGD("EV_bentoHot:res=%d",res);
    return res;
}

/*********************************************************************************************************
** Function name	:		EV_columnOpen
** Descriptions		:		普通柜开门接口
** input parameters	:       req 请求包结构体指针 rpt 回应包结构体指针
** output parameters:		无
** Returned value	:		0通信失败  1通信成功
*********************************************************************************************************/
int32  EV_API  EV_columnOpen(ST_COL_OPEN_REQ *req,ST_COL_OPEN_RPT *rpt)
{
    int32 res;
    EV_LOGD("EV_APIcolumnOpen:req=fd=%d,addr=%d,no=%d,goc=%d\n",(req)->fd,(req)->addr,(req)->no,(req)->goc);
    res = Column_open(req,rpt);
    EV_LOGD("EV_columnOpen:res=%d,result=%02x",res,rpt->result);
    return res;
}





/*********************************************************************************************************
** Function name	:		EV_bentoOpen
** Descriptions		:		BENTO快递柜开门接口
** input parameters	:       req 请求包结构体指针 rpt 回应包结构体指针
** output parameters:		无
** Returned value	:		0通信失败  1通信成功
*********************************************************************************************************/
int32  EV_API  EV_liftVendout(ST_COL_OPEN_REQ *req,ST_COL_OPEN_RPT *rpt)
{
    int32 res;
    EV_LOGD("EV_liftVendout:req=%x\n",(unsigned int)req);
    res = LIFT_vendout(req,rpt);
    EV_LOGD("EV_liftVendout:res=%d",res);
    return res;
}






