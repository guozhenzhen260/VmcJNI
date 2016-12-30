/*==========================================================================
##                   DB_driver
##
##              Copyright(c) 2015-2016 Yoc All rights reserved.
##
##--------------------------------------------------------------------------
## File Description
## ----------------
##      db_serial.c 串口驱动接口c文件
##
##--------------------------------------------------------------------------
##
##  Created by	:	Yoc
##			QQ	:	1760668483
##        Date	:	2015.05.20
##============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "db_serial.h"
#include "yoc_serialport.h"
#include "timer.h"
#include "ev_config.h"



int32 DB_getCh(int32 fd,char *ch)
{
    int i = 0;
#ifdef EV_WIN32
    i = yserial_read(fd,ch,1);
#else
    if(yserial_bytesAvailable(fd) > 0){
        i = yserial_read(fd,ch,1);
    }
#endif
    // EV_LOGD("DB_getCh:i=%d,ch=%d",i,*ch);
    return i;
}





