#ifndef _WIN_YOCSERIALPORT_H_
#define _WIN_YOCSERIALPORT_H_
#include "yoc_serialbase.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <termios.h>
#include <sys/ioctl.h>



int unixserial_open(char *portName);
void unixserial_setBaudRate(int fd, BaudRateType baudRate);
int unixserial_setTimeout(int fd,long millisec);
int unixserial_setFlowControl(int fd,FlowType flow);
int unixserial_setStopBits(int fd,StopBitsType stopBits);
int unixserial_setParity(int fd,ParityType parity);
int unixserial_setDataBits(int fd,int databits);
uint32 unixserial_bytesAvailable(int fd);
void unixserial_close(int fd);
uint32 unixserial_read (int fd,char *pData,uint32 len);
uint32 unixserial_write (int fd,const char* pData, uint32 len);
void unixserial_clear(int fd);


#endif
