#ifndef _WIN_YOCSERIALPORT_H_
#define _WIN_YOCSERIALPORT_H_
#include <windows.h>
#include "yoc_serialbase.h"


HANDLE winserial_open(char *portName);
void winserial_close(HANDLE win_handle);
uint32 winserial_read (HANDLE win_handle,char *pData,uint32 len);
uint32 winserial_write (HANDLE win_handle,const char* pData, uint32 len);
int winserial_setRWBuffer(HANDLE win_handle,DWORD dwInQueue,DWORD dwOutQueue);
uint32 winserial_bytesAvailable(HANDLE win_handle);
void winserial_setBaudRate(HANDLE win_handle, BaudRateType baudRate);
void winserial_setDataBits(HANDLE win_handle, DataBitsType dataBits);
void winserial_setStopBits(HANDLE win_handle,StopBitsType stopBits);
void winserial_setParity(HANDLE win_handle,ParityType parity);
void winserial_setFlowControl(HANDLE win_handle,FlowType flow);
void winserial_setTimeout(HANDLE win_handle,long millisec);
void winserial_clear(HANDLE win_handle);
#endif
