#include "freak_rs_api.h"
#include "ev_config.h"
#include "db_serial.h"
#include "timer.h"
#include "yoc_serialport.h"

static FRS_crc(uint8 *msg,int32 len)
{
    uint8 i,crc;
    for(i = 0,crc = 0;i < len;i++){
        crc += msg[i];
    }
    return crc;
}


static void FRS_LOG(uint8 type,uint8 *data,uint8 len)
{
    uint8 i,buf[512] = {0};
    for(i = 0;i < len;i++){
        sprintf((char *)&buf[i * 3],"%02x ",data[i]);
    }
    if(type == 1){
        EV_LOGD("FRS-Send[%d]:%s",len,buf);
    }
    else{
        EV_LOGI("FRS-Recv[%d]:%s\n",len,buf);
    }
}

int32 FRS_send(int32 fd,int32 port,char *data,int32 len)
{
    uint8 i,index;
    uint8 buf[FRS_BUF_SIZE] = {0};
    index = 0;
    buf[index++] = 0xFF;
    buf[index++] = 0xFF;
    buf[index++] = port;
    buf[index++] = 0x00;

    for(i = 0;i < len;i++){
        if(data[i] == 0xFF){
           buf[index++] =  0xFF;
           buf[index++] =  0xFE;
        }
        else{
           buf[index++] =  data[i];
        }
    }
    buf[3] = index - 4;
    buf[index] = FRS_crc(buf,index);
    index++;

    FRS_LOG(1,buf,index);

    return yserial_write(fd,buf,index);

}


int32 FRS_recv(int32 fd,int32 *port,char *data,int32 *rlen,int32 timeout)
{
    uint8 ch,index;
    uint8 crc,no,i;
    uint8 len = 0;
    int32 r;
    uint8 recvbuf[FRS_BUF_SIZE] = {0};
    index = 0;
    while(timeout > 0){
        r = DB_getCh(fd,(char *)&ch);
        if(r <= 0){
            EV_msleep(50);
            timeout = (timeout <= 50) ? 0 : timeout - 50;
            continue;
        }

        EV_LOGD("FRS_R:%x\n",ch);
        if(index == 0){
            if(ch == 0xFF){
                recvbuf[index++] = ch;
                timeout += 200;
            }
            else{
                index = 0;
            }
        }
        else if(index == 1){
            if(ch == 0xFF){
                recvbuf[index++] = ch;
            }
            else{
                index = 0;
            }
        }
        else if(index == 2){
            no = ch;
            recvbuf[index++] = ch;
        }
        else if(index == 3){
           len = ch;
           recvbuf[index++] = ch;
        }
        else{
            if(index >= (len + 4)){
                recvbuf[index++] = ch;
                crc  = FRS_crc(recvbuf,len + 4);
                FRS_LOG(2,recvbuf,index);
                if(crc == recvbuf[index - 1]){
                    *port = no;
                    for(i = 0;i < len;i++){
                        data[i] = recvbuf[4 + i];
                    }
                    *rlen = len;
                    return 1;
                }
                else{
                    FRS_LOG(2,recvbuf,index);
                    return 2;
                }
            }
            else{
                recvbuf[index++] = ch;
            }
        }
    }
    FRS_LOG(2,recvbuf,index);
    return 0;
}



int32 FRS_test(int32 fd)
{

    uint8 buf[] = {0xFF ,0xFF ,0x01 ,0x11 ,0x2A ,0x01 ,0x02 ,0x01 ,0x01 ,0x02 ,0x03 ,
                   0x04 ,0x05 ,0x06 ,0x07 ,0x08 ,0x09 ,0xFF ,0xFE ,0xFF ,0xFE ,0x65};

    EV_LOGD("FRS_send:\n");

    yserial_write(fd,buf,sizeof(buf));

    EV_LOGD("FRS_recv:\n");
}
