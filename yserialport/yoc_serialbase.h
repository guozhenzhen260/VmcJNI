#ifndef _YOC_SERIALBASE_H_
#define _YOC_SERIALBASE_H_

/*************************************************************************
** 目标机32位 定义
**************************************************************************/
typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef unsigned int        uint32;
typedef unsigned long       uint64;
typedef  char               int8;
typedef  short              int16;
typedef  int                int32;
typedef  long               int64;





typedef enum _baudrate_type_{
    BAUD50     = 50,                //POSIX ONLY
    BAUD75     = 75,                //POSIX ONLY
    BAUD110    = 110,
    BAUD134    = 134,               //POSIX ONLY
    BAUD150    = 150,               //POSIX ONLY
    BAUD200    = 200,               //POSIX ONLY
    BAUD300    = 300,
    BAUD600    = 600,
    BAUD1200   = 1200,
    BAUD1800   = 1800,              //POSIX ONLY
    BAUD2400   = 2400,
    BAUD4800   = 4800,
    BAUD9600   = 9600,
    BAUD14400  = 14400,             //WINDOWS ONLY
    BAUD19200  = 19200,
    BAUD38400  = 38400,
    BAUD56000  = 56000,             //WINDOWS ONLY
    BAUD57600  = 57600,
    BAUD76800  = 76800,             //POSIX ONLY
    BAUD115200 = 115200,
    BAUD128000 = 128000,            //WINDOWS ONLY
    BAUD256000 = 256000             //WINDOWS ONLY
}BaudRateType;

typedef enum _databits_type_ {
    DATA_5 = 5,
    DATA_6 = 6,
    DATA_7 = 7,
    DATA_8 = 8
}DataBitsType;

typedef enum _parity_type_ {
    PAR_NONE ,
    PAR_ODD  ,
    PAR_EVEN,
    PAR_MARK,               //WINDOWS ONLY
    PAR_SPACE
}ParityType;

typedef enum _stopbits_type_{
    STOP_1  = 1,
    STOP_1_5 ,               //WINDOWS ONLY
    STOP_2
}StopBitsType;

typedef enum _flow_type_
{
    FLOW_OFF,
    FLOW_HARDWARE,
    FLOW_XONXOFF
}FlowType;


/**
 * structure to contain port settings
 */

typedef struct _port_settings_{
    BaudRateType    BaudRate;
    DataBitsType    DataBits;
    ParityType      Parity;
    StopBitsType    StopBits;
    FlowType        FlowControl;
    long            Timeout_Millisec;
}PortSettings;










#endif
