/*==========================================================================
##                   DB_driver
##
##              Copyright(c) 2015-2016 Yoc All rights reserved.
##
##--------------------------------------------------------------------------
## File Description
## ----------------
##      mdb_api.h MDB驱动协议接口头文件
##
##--------------------------------------------------------------------------
##
##  Created by	:	Yoc
##			QQ	:	1760668483
##        Date	:	2015.05.20
##============================================================================*/
#ifndef _MDB_API_H_
#define _MDB_API_H_

#include "db_driver.h"

//DB --> PC
#define     SF          0
#define     LEN         1
#define     ADDR        2
#define     SN          3
#define     MT          4
#define     DATA        5

#define     HEAD_LEN    5

#define     HEAD_PC     0xE5
#define     HEAD_DB     0xE6

#define		DB_CON_ADDR         0xFF

#define 	DB_MT_CON_REQ		0x05
#define 	DB_MT_CON_RPT		0x75

#define 	DB_MT_ID_REQ		0x06
#define 	DB_MT_ID_RPT		0x76


#define		DB_MT_CHECK_REQ		0x21
#define		DB_MT_CHECK_RPT		0x91



#define 	MDB_CHECK_REQ 		0x01
#define 	MDB_CHECK_RPT 		0x01

#define 	MDB_BILL_INFO_REQ 	0x02
#define 	MDB_BILL_INFO_RPT	0x02

#define 	MDB_COIN_INFO_REQ 	0x03
#define 	MDB_COIN_INFO_RPT	0x03

#define 	MDB_INFO_REQ 		0x04
#define 	MDB_INFO_RPT		0x04


#define 	DB_MT_ACTION_REQ	0x22
#define 	DB_MT_ACTION_RPT	0x92
#define 	MDB_INIT_REQ 		0x01
#define 	MDB_INIT_RPT		0x01
#define 	MDB_CONTROL_REQ 	0x02
#define 	MDB_CONTROL_RPT		0x02
#define 	MDB_PAYOUT_REQ 		0x03
#define 	MDB_PAYOUT_RPT		0x03
#define 	MDB_PAYBACK_REQ 	0x04
#define 	MDB_PAYBACK_RPT		0x04
#define 	MDB_COST_REQ 		0x05
#define 	MDB_COST_RPT		0x05
#define 	MDB_COIN_CON_REQ 		0x06
#define 	MDB_COIN_CON_RPT		0x06
#define 	MDB_BILL_CON_REQ 		0x07
#define 	MDB_BILL_CON_RPT		0x07
#define 	MDB_HP_PAYOUT_REQ 		0x08
#define 	MDB_HP_PAYOUT_RPT		0x08

#define     MDB_ADDR            0x31

#define 	OBJ_BILL		0x01
#define 	OBJ_COIN		0x02

#define     BILL_ACCEPTOR_NONE		0
#define     BILL_ACCEPTOR_MDB		2

#define     BILL_DISPENSER_NONE		0
#define     BILL_DISPENSER_MDB		2

#define     COIN_ACCEPTOR_NONE		0
#define     COIN_ACCEPTOR_PPLUSE	1
#define     COIN_ACCEPTOR_MDB		2
#define     COIN_ACCEPTOR_SPLUSE	3


#define     COIN_DISPENSER_NONE		0
#define     COIN_DISPENSER_HOPPER	1
#define     COIN_DISPENSER_MDB		2


#define COIN_BIT_FAULT						0x01UL
#define COIN_BIT_DISABLE					(0x01UL << 1)
#define COIN_BIT_QUEBI						(0x01UL << 2)
#define COIN_BIT_BUSY						(0x01UL << 3)
#define COIN_BIT_RESET						(0x01UL << 4)
#define COIN_BIT_HOPPER						(0x01UL << 5)
#define COIN_BIT_OK							(0x01UL << 6)
#define COIN_BIT_PAYBACK					(0x01UL << 7)


#define COIN_ERR_COM					0x0001UL
#define COIN_ERR_SENSOR					0x0002UL
#define COIN_ERR_TUBEJAM				0x0004UL
#define COIN_ERR_ROM					0x0008UL
#define COIN_ERR_ROUTING				0x0010UL
#define COIN_ERR_JAM					0x0020UL
#define COIN_ERR_REMOVETUBE				0x0040UL
//#define COIN_ERR_DISABLE				0x0100UL
#define COIN_ERR_UNKNOW					0x8000UL


#define BILL_BIT_FAULT						(0x01UL) 		  //¹ÊÕÏ
#define BILL_BIT_DISABLE					(0x01UL << 1) //½ûÄÜÎ»
#define BILL_BIT_QUEBI						(0x01UL << 2) //È±±ÒÎ»
#define BILL_BIT_BUSY						(0x01UL << 3) //Ó²±ÒÆ÷Ã¦Î»
#define BILL_BIT_RESET						(0x01UL << 4)
#define BILL_BIT_PAYBACK					(0x01UL << 7) //ÍË±ÒÎ»




#define BILL_ERR_COM					0x0001UL //Í¨ÐÅ¹ÊÕÏ
#define BILL_ERR_SENSOR					0x0002UL //´«¸ÐÆ÷¹ÊÕÏ
#define BILL_ERR_TUBEJAM				0x0004UL //³ö±Ò¿Ú¿¨±Ò
#define BILL_ERR_ROM					0x0008UL //rom³ö´í
#define BILL_ERR_ROUTING				0x0010UL //½ø±ÒÍ¨µÀ³ö´í
#define BILL_ERR_JAM					0x0020UL //Í¶±Ò¿¨±Ò
#define BILL_ERR_REMOVECASH				0x0040UL //ÒÆ³ýÖ½±Ò³®Ïä
#define BILL_ERR_DISABLE				0x0080UL //½ûÄÜ
#define BILL_ERR_MOTO					0x0100UL //Âí´ï¹ÊÕÏ
#define BILL_ERR_CASH					0x0200UL //Ö½±Ò³®Ïä¹ÊÕÏ
#define BILL_ERR_UNKNOW					0x8000UL //ÆäËû¸÷ÖÖ¹ÊÕÏ





int32 MDB_heart_check(ST_MDB_HEART_REQ *req,ST_MDB_HEART_RPT *rpt);
int32 MDB_coin_info_check(const ST_MDB_COIN_INFO_REQ *req,ST_MDB_COIN_INFO_RPT *rpt);
int32 MDB_bill_info_check(const ST_MDB_BILL_INFO_REQ *req,ST_MDB_BILL_INFO_RPT *rpt);
int32 MDB_init(ST_MDB_INIT_REQ *req,ST_MDB_INIT_RPT *rpt);
int32 MDB_enable(ST_MDB_ENABLE_REQ *req,ST_MDB_ENABLE_RPT *rpt);
int32 MDB_cost(const ST_MDB_COST_REQ *req,ST_MDB_COST_RPT *rpt);
int32 MDB_payback(const ST_MDB_PAYBACK_REQ *req,ST_MDB_PAYBACK_RPT *rpt);
int32 MDB_payout(const ST_MDB_PAYOUT_REQ *req,ST_MDB_PAYOUT_RPT *rpt);
int32 MDB_bill_config(const ST_MDB_BILL_CON_REQ *req,ST_MDB_BILL_CON_RPT *rpt);
int32 MDB_coin_config(const ST_MDB_COIN_CON_REQ *req,ST_MDB_COIN_CON_RPT *rpt);
int32 MDB_hpPayout(const ST_MDB_HP_PAYOUT_REQ *req,ST_MDB_HP_PAYOUT_RPT *rpt);
#endif
