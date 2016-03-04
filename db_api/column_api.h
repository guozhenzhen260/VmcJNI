#ifndef COLUMN_API_H
#define COLUMN_API_H
#include "db_driver.h"

#define COL_SEND_HEAD      0xED
#define COL_SEND_LEN       0x08
#define COL_SEND_NONE	   0x00


#define COL_SEND_OPEN 	   0x70
#define COL_SEND_CHECK     0x71

#define COL_RECV_HEAD      0xEE




int32 Column_open(const ST_COL_OPEN_REQ *req,ST_COL_OPEN_RPT *rpt);
//int32  BT_check(const ST_BT_CHECK_REQ *req,ST_BT_CHECK_RPT *rpt);

#endif // BENTO_API_H
