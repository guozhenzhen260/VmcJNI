#include "win_yocSerialPort.h"

/*********************************************************************************************************
** Function name:     	winserial_open
** Descriptions:	    串口打开函数
** input parameters:    portName 串口文件路径
** output parameters:   无
** Returned value:      成功返回win32文件句柄  失败返回 NULL
*********************************************************************************************************/
HANDLE winserial_open(char *portName)
{
    HANDLE Win_Handle;
    COMMCONFIG Win_CommConfig;//串口配置结构体
    long confSize = sizeof(COMMCONFIG);

    Win_Handle=CreateFileA(portName, GENERIC_READ|GENERIC_WRITE,
                          FILE_SHARE_READ|FILE_SHARE_WRITE, NULL,
                          OPEN_EXISTING, 0, NULL);
    if (Win_Handle!=INVALID_HANDLE_VALUE)
    {
        /*configure port settings*/
        GetCommConfig(Win_Handle, (LPCOMMCONFIG)&Win_CommConfig, (LPDWORD)&confSize);
        GetCommState(Win_Handle, &(Win_CommConfig.dcb));

        /*set up parameters*/
        Win_CommConfig.dcb.fBinary=TRUE;
        Win_CommConfig.dcb.fInX=FALSE;
        Win_CommConfig.dcb.fOutX=FALSE;
        Win_CommConfig.dcb.fAbortOnError=FALSE;
        Win_CommConfig.dcb.fNull=FALSE;
        SetCommConfig(Win_Handle, &Win_CommConfig, sizeof(COMMCONFIG));
        return Win_Handle;
    }

    return NULL;

}

void winserial_setTimeout(HANDLE win_handle,long millisec)
{

    COMMTIMEOUTS Win_CommTimeouts;
    if (millisec == -1) {
        Win_CommTimeouts.ReadIntervalTimeout = MAXDWORD;
        Win_CommTimeouts.ReadTotalTimeoutConstant = 0;
    } else {
        Win_CommTimeouts.ReadIntervalTimeout = millisec;
        Win_CommTimeouts.ReadTotalTimeoutConstant = millisec;
    }
    Win_CommTimeouts.ReadTotalTimeoutMultiplier = 0;
    Win_CommTimeouts.WriteTotalTimeoutMultiplier = millisec;
    Win_CommTimeouts.WriteTotalTimeoutConstant = 0;
    SetCommTimeouts(win_handle, &Win_CommTimeouts);
}



void winserial_setFlowControl(HANDLE win_handle,FlowType flow)
{
    COMMCONFIG config;//串口配置结构体
    long conSize = sizeof(COMMCONFIG);

    GetCommConfig(win_handle, (LPCOMMCONFIG)&config, (LPDWORD)&conSize);
    GetCommState(win_handle, &(config.dcb));

    switch(flow) {

        /*no flow control*/
        case FLOW_OFF:
            config.dcb.fOutxCtsFlow=FALSE;
            config.dcb.fRtsControl=RTS_CONTROL_DISABLE;
            config.dcb.fInX=FALSE;
            config.dcb.fOutX=FALSE;
            SetCommConfig(win_handle, &config, sizeof(COMMCONFIG));
            break;

        /*software (XON/XOFF) flow control*/
        case FLOW_XONXOFF:
            config.dcb.fOutxCtsFlow=FALSE;
            config.dcb.fRtsControl=RTS_CONTROL_DISABLE;
            config.dcb.fInX=TRUE;
            config.dcb.fOutX=TRUE;
            SetCommConfig(win_handle, &config, sizeof(COMMCONFIG));
            break;

        case FLOW_HARDWARE:
            config.dcb.fOutxCtsFlow=TRUE;
            config.dcb.fRtsControl=RTS_CONTROL_HANDSHAKE;
            config.dcb.fInX=FALSE;
            config.dcb.fOutX=FALSE;
            SetCommConfig(win_handle, &config, sizeof(COMMCONFIG));
            break;
        default:break;
    }
}



void winserial_setParity(HANDLE win_handle,ParityType parity)
{
    COMMCONFIG config;//串口配置结构体
    long conSize = sizeof(COMMCONFIG);

    GetCommConfig(win_handle, (LPCOMMCONFIG)&config, (LPDWORD)&conSize);
    GetCommState(win_handle, &(config.dcb));
    if(config.dcb.Parity  != parity)
    {
        config.dcb.Parity = parity;
        SetCommConfig(win_handle, &config, sizeof(COMMCONFIG));
    }
}



void winserial_setStopBits(HANDLE win_handle,StopBitsType stopBits)
{
    COMMCONFIG config;//串口配置结构体
    long conSize = sizeof(COMMCONFIG);

    GetCommConfig(win_handle, (LPCOMMCONFIG)&config, (LPDWORD)&conSize);
    GetCommState(win_handle, &(config.dcb));
    if(config.dcb.StopBits  != stopBits)
    {
        config.dcb.StopBits = stopBits;
        SetCommConfig(win_handle, &config, sizeof(COMMCONFIG));
    }
}


void winserial_setDataBits(HANDLE win_handle, DataBitsType dataBits) {

    COMMCONFIG config;//串口配置结构体
    long conSize = sizeof(COMMCONFIG);

    GetCommConfig(win_handle, (LPCOMMCONFIG)&config, (LPDWORD)&conSize);
    GetCommState(win_handle, &(config.dcb));
    if(config.dcb.ByteSize  != dataBits)
    {
        config.dcb.ByteSize = dataBits;
        SetCommConfig(win_handle, &config, sizeof(COMMCONFIG));
    }

}




/*********************************************************************************************************
** Function name:     	winserial_setBaudRate
** Descriptions:	    设置波特率
** input parameters:
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/
void winserial_setBaudRate(HANDLE win_handle, BaudRateType baudRate)
{
    COMMCONFIG config;//串口配置结构体
    long conSize = sizeof(COMMCONFIG);
    uint32 baud;
    switch (baudRate)
    {
        case BAUD50:
        case BAUD75:
        case BAUD134:
        case BAUD150:
        case BAUD200:
            baud = BAUD110;
            break;
        case BAUD1800:
            baud = BAUD1200;
            break;
        case BAUD76800:
            baud = BAUD57600;
            break;
        default:
            baud = baudRate;
            break;
    }

    /*configure port settings*/
    GetCommConfig(win_handle, (LPCOMMCONFIG)&config, (LPDWORD)&conSize);
    GetCommState(win_handle, &(config.dcb));
    if(config.dcb.BaudRate != baud)
    {
       config.dcb.BaudRate = baud;
       SetCommConfig(win_handle, &config, sizeof(COMMCONFIG));
    }


}


uint32 winserial_bytesAvailable(HANDLE win_handle)
{
    DWORD Errors;
    COMSTAT Status;
    BOOL success=ClearCommError(win_handle, &Errors, &Status);
    if (success) {
        return Status.cbInQue;
    }
    return 0;
}


int winserial_setRWBuffer(HANDLE win_handle,DWORD dwInQueue,DWORD dwOutQueue)
{

    if (!SetupComm(win_handle,dwInQueue,dwOutQueue))
    {
        //errStr = "SetRWBuffer:Unable to setup the COM-port";// Display a warning
        winserial_close(win_handle);// Close the port
        return 0;
    }
    return 1;
}



void winserial_close(HANDLE win_handle)
{
    CloseHandle(win_handle);
}



uint32 winserial_read (HANDLE win_handle,char *pData,uint32 len)
{
    DWORD dwRead;
    if (!ReadFile(win_handle,(void*)pData,(DWORD)len,&dwRead,NULL))
    {
        return 0;
    }
    return dwRead;
}


uint32 winserial_write (HANDLE win_handle,const char* pData, uint32 len)
{
    DWORD dwWritten;
    DWORD err;
    COMSTAT stat;
    ClearCommError(win_handle,&err,&stat);
    if(err)
    {
        //errStr = "Write:Serial err !";
        return 0;
    }
    // Write the data
    if (!WriteFile(win_handle,(void *)pData,(DWORD)len,&dwWritten,NULL))
    {
        //errStr = "Write:Unable to write the data";
        return 0;
    }
    return dwWritten;
}


void winserial_clear(HANDLE win_handle)
{
    PurgeComm(win_handle,PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR | PURGE_RXCLEAR);
}





