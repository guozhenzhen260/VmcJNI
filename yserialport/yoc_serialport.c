#include "yoc_serialport.h"
#include "ev_config.h"

typedef struct _st_port{
#if ( defined EV_WIN32 )
    HANDLE  fd;
#else
    int     fd;
#endif
    int     id;   //提供给上层的串口ID号
    char    *portName; //串口号名称
}ST_PORT;


#define MAX_PORT    100
static ST_PORT *port_arr[MAX_PORT] = {NULL};


ST_PORT *yserialPort(int32 fd){
   // EV_LOGD("yserialPort:fd = %d\n",fd);
    if(fd >= 0 && fd < MAX_PORT){
        return port_arr[fd];
    }
    else{
        return NULL;
    }

}




char *yserial_getPortName(int32 fd)
{
    ST_PORT *port = yserialPort(fd);
    if(port == NULL){return NULL;}
    return port->portName;
}





ST_PORT *yserialCreate(char *portName){
    ST_PORT *port = NULL;
    int i,len;
    //分配内存
    port = malloc(sizeof(ST_PORT));
    if(port == NULL){
        return NULL;
    }

    len = strlen(portName);
    port->portName = malloc(len + 2); //分配内存多分配一个字节用于存储结束符
    if(port->portName == NULL){
        free(port);
        port = NULL;
        return NULL;
    }
    memset(port->portName,0,len + 2);
    strcpy(port->portName,portName);
    for(i = 0;i < MAX_PORT;i++){
        if(port_arr[i] == NULL){
            port->id = i;
            port_arr[i] = port;
            return port;
        }
    }

    free(port->portName);
    free(port);
    return NULL;
}




void yserial_close(int32 fd)
{
    ST_PORT *port = yserialPort(fd);
    if(port == NULL){
        return;
    }
#if ( defined EV_WIN32 )
    winserial_close(port->fd);
#else  //linux待完善
    unixserial_close(port->fd);
#endif
    //释放资源
    port_arr[port->id] = NULL;
    free(port->portName);
    free(port);
}


void yserial_closeByName(char *portName)
{
	int i ;
	ST_PORT *port;
	
	for(i = 0;i < MAX_PORT;i++){
		port = port_arr[i];
		if(strcmp(port->portName,portName) == 0){
			yserial_close(port->id);
		}
	}
}


int32 yserial_open(char *portName)
{
    ST_PORT *port;
    port = yserialCreate(portName);
    if(port == NULL){
        EV_LOGE("yserial_open:port = NULL\n");
        return -1;
    }
#if ( defined EV_WIN32 )
    port->fd  = winserial_open(portName);
    if(port->fd == NULL){
        port_arr[port->id] = NULL;
        free(port->portName);
        free(port);
        return -1;
    }
    winserial_setRWBuffer(port->fd,1024,1024);
    winserial_setBaudRate(port->fd,BAUD9600);
    winserial_setDataBits(port->fd,DATA_8);
    winserial_setStopBits(port->fd,STOP_1);
    winserial_setParity(port->fd,PAR_NONE);
    winserial_setFlowControl(port->fd,FLOW_OFF);
    winserial_setTimeout(port->fd,10);
#else
    port->fd = unixserial_open(portName);
    if(port->fd == -1){
        port_arr[port->id] = NULL;
        free(port->portName);
        free(port);
        return -1;
    }
    unixserial_setBaudRate(port->fd,BAUD9600);
    unixserial_setDataBits(port->fd,DATA_8);
    unixserial_setStopBits(port->fd,STOP_1);
    unixserial_setParity(port->fd,PAR_NONE);
    unixserial_setFlowControl(port->fd,FLOW_OFF);
    unixserial_setTimeout(port->fd,10);
    EV_LOGD("yserial_open:fd=%d id=%d",port->fd,port->id);
#endif
    return port->id;
}



uint32 yserial_read (int fd,char *pData,uint32 len)
{
    ST_PORT *port = yserialPort(fd);
    if(port == NULL){
        return 0;
    }
#if ( defined EV_WIN32 )
    return winserial_read(port->fd,pData,len);
#else
    return unixserial_read(port->fd,pData,len);
#endif
}


uint32 yserial_write (int fd,const char* pData, uint32 len)
{
    ST_PORT *port = yserialPort(fd);
    if(port == NULL){
        return 0;
    }
#if ( defined EV_WIN32 )
    return winserial_write(port->fd,pData,len);
#else
    return unixserial_write(port->fd,pData,len);
#endif
}


int yserial_setRWBuffer(int fd,uint32 dwInQueue,uint32 dwOutQueue)
{
    ST_PORT *port = yserialPort(fd);
    if(port == NULL){
        return 0;
    }
#if ( defined EV_WIN32 )
    return winserial_setRWBuffer(port->fd,(DWORD)dwInQueue,(DWORD)dwOutQueue);
#else
    return 1;
#endif
}



uint32 yserial_bytesAvailable(int fd)
{
    ST_PORT *port = yserialPort(fd);
    if(port == NULL){
        return 0;
    }
#if ( defined EV_WIN32 )
    return winserial_bytesAvailable(port->fd);
#else
    //EV_LOGD("yserial_bytesAvailable:fd=%d,i=%d",port->fd,i);
    return unixserial_bytesAvailable(port->fd);
    //EV_LOGD("yserial_bytesAvailable1:fd=%d,i=%d",port->fd,i);
   // return i;
#endif
}


void yserial_setBaudRate(int fd, BaudRateType baudRate)
{
    ST_PORT *port = yserialPort(fd);
    if(port == NULL){
        return;
    }
#if ( defined EV_WIN32 )
    winserial_setBaudRate(port->fd,baudRate);
#else
    unixserial_setBaudRate(port->fd,baudRate);
#endif
}



void yserial_setDataBits(int fd, DataBitsType dataBits)
{
    ST_PORT *port = yserialPort(fd);
    if(port == NULL){
        return ;
    }
#if ( defined EV_WIN32 )
    winserial_setDataBits(port->fd,dataBits);
#else
    unixserial_setDataBits(port->fd,dataBits);
#endif
}


void yserial_setStopBits(int fd,StopBitsType stopBits)
{
    ST_PORT *port = yserialPort(fd);
    if(port == NULL){
        return;
    }
#if ( defined EV_WIN32 )
    winserial_setStopBits(port->fd,stopBits);
#else
    unixserial_setStopBits(port->fd,stopBits);
#endif
}



void yserial_setParity(int fd,ParityType parity)
{
    ST_PORT *port = yserialPort(fd);
    if(port == NULL){
        return;
    }
#if ( defined EV_WIN32 )
    winserial_setParity(port->fd,parity);
#else
    unixserial_setParity(port->fd,parity);
#endif
}



void yserial_setFlowControl(int fd,FlowType flow)
{
    ST_PORT *port = yserialPort(fd);
    if(port == NULL){
        return ;
    }
#if ( defined EV_WIN32 )
    winserial_setFlowControl(port->fd,flow);
#else
    unixserial_setFlowControl(port->fd,flow);
#endif
}



void yserial_setTimeout(int fd,long millisec)
{
    ST_PORT *port = yserialPort(fd);
    if(port == NULL){
        return ;
    }
#if ( defined EV_WIN32 )
    winserial_setTimeout(port->fd,millisec);
#else
    unixserial_setTimeout(port->fd,millisec);
#endif




}



void yserial_clear(int fd)
{
    ST_PORT *port = yserialPort(fd);
    if(port == NULL){
        return ;
    }
#if ( defined EV_WIN32 )
    winserial_clear(port->fd);
#else
    unixserial_clear(port->fd);
#endif
}
