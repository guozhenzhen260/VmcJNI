#include "unix_yocSerialPort.h"
#include "fcntl.h"


/*********************************************************************************************************
** Function name:     	unixserial_open
** Descriptions:	    串口打开函数
** input parameters:    portName 串口文件路径
** output parameters:   无
** Returned value:      成功返回文件描述符  失败返回- 1
*********************************************************************************************************/
int unixserial_open(char *portName)
{
    //O_RDWR|O_NOCTTY|O_NDELAY
    int fd = open(portName, O_RDWR);//阻塞方式
    if (fd < 0)
    {
        printf("Can't Open Serial Port:%s\n",portName);
        return (-1);
    }
    return fd;

}



int unixserial_setTimeout(int fd,long millisec)
{
    struct termios options;
    struct timeval timeout;
    if ( tcgetattr( fd,&options)  !=  0)
    {
        perror("SetupSerial 1");
        close(fd);
        return(0);
    }
    timeout.tv_sec = millisec / 1000;
    timeout.tv_usec = millisec % 1000;
    options.c_cc[VTIME] = millisec/100;
    tcsetattr(fd, TCSAFLUSH, &options);
    return 1;
}



int unixserial_setFlowControl(int fd,FlowType flow)
{
    struct termios options;
    if ( tcgetattr( fd,&options)  !=  0)
    {
        perror("SetupSerial 1");
        close(fd);
        return(0);
    }
    switch(flow) {
        /*no flow control*/
        case FLOW_OFF:
            options.c_cflag&=(~CRTSCTS);
            options.c_iflag&=(~(IXON|IXOFF|IXANY));
            tcsetattr(fd, TCSAFLUSH, &options);
            break;

        /*software (XON/XOFF) flow control*/
        case FLOW_XONXOFF:
            options.c_cflag&=(~CRTSCTS);
            options.c_iflag|=(IXON|IXOFF|IXANY);
            tcsetattr(fd, TCSAFLUSH, &options);
            break;

        case FLOW_HARDWARE:
            options.c_cflag|=CRTSCTS;
            options.c_iflag&=(~(IXON|IXOFF|IXANY));
            tcsetattr(fd, TCSAFLUSH, &options);
            break;
    }

    return 1;

}


int unixserial_setStopBits(int fd,StopBitsType stopBits)
{

    struct termios options;
    if ( tcgetattr( fd,&options)  !=  0)
    {
        perror("SetupSerial 1");
        close(fd);
        return(0);
    }
    switch (stopBits) {
        /*one stop bit*/
        case STOP_1:
            options.c_cflag&=(~CSTOPB);
            tcsetattr(fd, TCSAFLUSH, &options);
            break;
        /*1.5 stop bits*/
        case STOP_1_5:
              break;
        /*two stop bits*/
        case STOP_2:
            options.c_cflag|=CSTOPB;
            tcsetattr(fd, TCSAFLUSH, &options);
            break;
    }
    return 1;

}



int unixserial_setParity(int fd,ParityType parity)
{

    struct termios options;
    if ( tcgetattr( fd,&options)  !=  0)
    {
        perror("SetupSerial 1");
        close(fd);
        return(0);
    }

    switch (parity) {
        /*space parity*/
        case PAR_SPACE:
            break;
        /*mark parity - WINDOWS ONLY*/
        case PAR_MARK:
            break;
        /*no parity*/
        case PAR_NONE:
            options.c_cflag&=(~PARENB);
            tcsetattr(fd, TCSAFLUSH, &options);
            break;
        /*even parity*/
        case PAR_EVEN:
            options.c_cflag&=(~PARODD);
            options.c_cflag|=PARENB;
            tcsetattr(fd, TCSAFLUSH, &options);
            break;
        /*odd parity*/
        case PAR_ODD:
            options.c_cflag|=(PARENB|PARODD);
            tcsetattr(fd, TCSAFLUSH, &options);
            break;
    }

    return 1;

}




/*********************************************************************************************************
** Function name:     	unixserial_setParity
** Descriptions:	    串口设置参数
** input parameters:    port 串口号
** output parameters:
** Returned value:      1:设置成功 0设置失败
*********************************************************************************************************/
int unixserial_setDataBits(int fd,int databits)
{
    struct termios options;

    if ( tcgetattr( fd,&options)  !=  0) {
        perror("SetupSerial 1");
        close(fd);
        return(0);
    }
    options.c_cflag &= ~CSIZE;
    switch (databits){
        case 6:
            options.c_cflag |= CS6;
            break;
        case 7:
            options.c_cflag |= CS7;
            break;
        case 8:
            options.c_cflag |= CS8;
            break;
        default:
            fprintf(stderr,"Unsupported data size\n");
            return (0);
    }

    tcflush(fd,TCIOFLUSH); /* Update the options and do it NOW */
    if (tcsetattr(fd,TCSANOW,&options) != 0){
        perror("SetupSerial 3");
        return (0);
    }
    return (1);
}



static speed_t getBaudrate(int baudrate)
{
    switch(baudrate) {
    case 0: return B0;
    case 50: return B50;
    case 75: return B75;
    case 110: return B110;
    case 134: return B134;
    case 150: return B150;
    case 200: return B200;
    case 300: return B300;
    case 600: return B600;
    case 1200: return B1200;
    case 1800: return B1800;
    case 2400: return B2400;
    case 4800: return B4800;
    case 9600: return B9600;
    case 19200: return B19200;
    case 38400: return B38400;
    case 57600: return B57600;
    case 115200: return B115200;
    case 230400: return B230400;
    case 460800: return B460800;
    case 500000: return B500000;
    case 576000: return B576000;
    case 921600: return B921600;
    case 1000000: return B1000000;
    case 1152000: return B1152000;
    case 1500000: return B1500000;
    case 2000000: return B2000000;
    case 2500000: return B2500000;
    case 3000000: return B3000000;
    case 3500000: return B3500000;
    case 4000000: return B4000000;
    default: return -1;
    }
}

/*********************************************************************************************************
** Function name:     	unixserial_setBaudRate
** Descriptions:	    设置波特率
** input parameters:
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/
void unixserial_setBaudRate(int fd, BaudRateType baudRate)
{
    int   status,res;
    speed_t speed;
    struct termios   Opt;

    speed = getBaudrate((int)baudRate);
    status = tcgetattr(fd, &Opt);
    tcflush(fd, TCIOFLUSH);
    cfmakeraw(&Opt);
    cfsetispeed(&Opt, speed);
    cfsetospeed(&Opt, speed);
    status = tcsetattr(fd, TCSANOW, &Opt);
    if (status != 0){
        perror("tcsetattr fd1");
    }
    tcflush(fd, TCIOFLUSH);
}


uint32 unixserial_bytesAvailable(int fd)
{
    static struct timeval timeout;
    fd_set fileSet;
    FD_ZERO(&fileSet);
    FD_SET(fd, &fileSet);
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    int n=select(fd + 1, &fileSet, NULL, NULL, &timeout);
    if (n <= 0) {
        return 0;
    }
    if(FD_ISSET(fd,&fileSet))//证明有数据可读
    {
        return 1;
    }
    return 0;
}


void unixserial_close(int fd)
{
    close(fd);
}



uint32 unixserial_read (int fd,char *pData,uint32 len)
{

    uint32 ret ;
    ret = read(fd , pData , len); //读取串口
    if(ret<0)
        return 0;
    return ret;
}


uint32 unixserial_write (int fd,const char* pData, uint32 len)
{
    uint32 written ;
    written = write(fd , pData , len);  //写串口
    if(written <0)
    {
        perror("write serial");
        return 0;
    }
    return  written;
}


void unixserial_clear(int fd)
{
    tcflush(fd,TCIOFLUSH);
}





