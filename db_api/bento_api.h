#ifndef BENTO_API_H
#define BENTO_API_H
#include "db_driver.h"

#define BT_HEAD           0xC7

#define BT_OPEN 			0x52
#define BT_CHECK          0x51
#define BT_LIGHT			0x56
#define BT_HOT 			0x53
#define BT_COOL 			0x55




#define BT_OPEN_ACK 		0x62
#define BT_CHECK_ACK 		0x61
#define BT_LIGHT_ACK		0x66
#define BT_HOT_ACK 		0x63
#define BT_COOL_ACK 		0x65



int32 BT_open(const ST_BT_OPEN_REQ *req,ST_BT_OPEN_RPT *rpt);
int32  BT_check(const ST_BT_CHECK_REQ *req,ST_BT_CHECK_RPT *rpt);
int32 BT_light(const ST_BT_LIGHT_REQ *req,ST_BT_LIGHT_RPT *rpt);
int32 BT_hot(const ST_BT_HOT_REQ *req,ST_BT_HOT_RPT *rpt);
int32 BT_cool(const ST_BT_COOL_REQ *req,ST_BT_COOL_RPT *rpt);
#endif // BENTO_API_H
