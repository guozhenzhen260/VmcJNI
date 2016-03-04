#ifndef _YOC_SERIALPORT_H_
#define _YOC_SERIALPORT_H_

#if ( defined EV_WIN32 )
#include "win_yocSerialPort.h"
#else
#include "unix_yocSerialPort.h"
#endif








int yserial_open(char *portName);
void yserial_close(int fd);
char *yserial_getPortName(int32 fd);
uint32 yserial_read (int fd,char *pData,uint32 len);
uint32 yserial_write (int fd,const char* pData, uint32 len);
int yserial_setRWBuffer(int fd,uint32 dwInQueue,uint32 dwOutQueue);
uint32 yserial_bytesAvailable(int fd);
void yserial_setBaudRate(int fd, BaudRateType baudRate);
void yserial_setDataBits(int fd, DataBitsType dataBits);
void yserial_setStopBits(int fd,StopBitsType stopBits);
void yserial_setParity(int fd,ParityType parity);
void yserial_setFlowControl(int fd,FlowType flow);
void yserial_setTimeout(int fd,long millisec);
void yserial_clear(int fd);




#endif
