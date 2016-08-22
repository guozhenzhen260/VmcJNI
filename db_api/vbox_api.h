#ifndef VBOX_API_H
#define VBOX_API_H
#include "db_driver.h"


//VMC-->PC
#define VBOX_ACK_RPT     0x01
#define VBOX_NAK_RPT     0x02
#define VBOX_POLL        0x03
#define VBOX_VMC_SETUP   0x05
#define VBOX_PAYIN_RPT   0x06
#define VBOX_PAYOUT_RPT  0x07
#define VBOX_HUODAO_RPT  0x0E
#define VBOX_VENDOUT_RPT 0x08
#define VBOX_INFO_RPT    0x11
#define VBOX_ACTION_RPT  0x0B
#define VBOX_BUTTON_RPT  0x0C
#define VBOX_STATUS_RPT  0x0D
#define VBOX_REQUEST    0x09
#define VBOX_ADMIN_RPT  0x0A
#define VBOX_COST_RPT   0x10


//PC-->VMC
#define VBOX_ACK            0x80
#define VBOX_NAK            0x81
#define VBOX_RESET_IND      0x84
#define VBOX_GET_SETUP      0x90
#define VBOX_GET_HUODAO     0x8A
#define VBOX_HUODAO_IND     0x87
#define VBOX_POSITION_IND   0x88
#define VBOX_SALEPRICE_IND  0x8E
#define VBOX_HUODAO_SET_IND 0x8F
#define VBOX_PRICE_SET_IND  0x8E
#define VBOX_VENDOUT_IND    0x83
#define VBOX_CONTROL_IND    0x85
#define VBOX_GET_INFO       0x8C
#define VBOX_GET_INFO_EXP   0x92
#define VBOX_SET_HUODAO     0x93
#define VBOX_GET_STATUS     0x86
#define VBOX_PAYOUT_IND     0x89
#define VBOX_COST_IND       0x8B


#define VBOX_TIMEOUT        0xFE
#define VBOX_DATA_ERROR     0xEE


#define VBOX_PC_REQ_IDLE         0  //空闲可以接受指令
#define VBOX_PC_REQ_SENDREADY    3 //准备发送
#define VBOX_PC_REQ_SENDING      1  //正在发送
#define VBOX_PC_REQ_HANDLING     2  //正在处理

#define VBOX_EV_TIMEROUT_VMC  10  //10秒超时
#define VBOX_EV_TIMEROUT_PC   10  //30秒超时
#define VBOX_EV_TIMEROUT_PC_LONG   120  //30秒超时

typedef struct{
    int32 port;
    uint8 addr;
    uint8 mt;
    uint8 sn;
    uint8 ver;
    uint8 F7;
    uint8 res;
    uint8 bin;
    uint8 row;
    uint8 column;
    uint8 sendbuf[256];
    uint8 sendlen;
    uint8 recvbuf[256];
    uint8 recvlen;
    uint8 data[256];
    uint8 datalen;

}VBOX_MSG;



typedef struct{
    uint8 port;
    uint8 device;
    uint8 id[100];
    uint8 id_len;
}VBOX_JSON_SP_ID;


typedef struct{
    uint8 port;
    uint8 device;
    uint16 price[100];
    uint8 price_len;
}VBOX_JSON_SP_PRICE;



VBOX_MSG *VBOX_readMsg(int32 port,uint32 timeout);
int VBOX_sendMsg(int32 port,VBOX_MSG *msg);
VBOX_MSG *VBOX_getMsg(void);
int VBOX_sendAck(int32 port);
int VBOX_sendNak(int32 port);
int VBOX_getSetup(int32 port);
int VBOX_getHuoDao(int32 port,int32 device);
int VBOX_getStatus(int32 port);
int VBOX_getInfo(int32 port,int32 type);
int VBOX_resetInd(int32 port,int32 dt);
int VBOX_controlInd(int32 port,int32 type,int32 value);
int VBOX_huodaoInd(int32 port,uint8 device,uint8 *buf,uint8 len);
int VBOX_positionInd(int32 port,uint8 device,uint8 *buf,uint8 len);
int VBOX_vendoutInd(int32 port,int32 device,int32 method,int32 id,int32 type,int32 cost);
int VBOX_payoutInd(int32 port,uint8 device,uint16 value,uint8 type);
int VBOX_costInd(int32 port,uint8 device,uint16 value,uint8 type);
int VBOX_salePriceInd(int32 port,uint8 device,uint16 *buf,uint8 len);
int VBOX_huodaoSetInd(int32 port,uint8 device,uint8 *buf,uint8 len);
#endif // VBOX_API_H
