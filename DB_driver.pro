##==========================================================================
##                   DB_driver
##
##              Copyright(c) 2015-2016 Yoc All rights reserved.
##
##--------------------------------------------------------------------------
## File Description
## ----------------
##      DB_driver.pro 货道与MDB驱动板库工程文件
##
##--------------------------------------------------------------------------
##
##  Created by	:	Yoc
##			QQ	:	1760668483
##        Date	:	2015.05.20
##============================================================================

TARGET = DB_driver
TEMPLATE = lib
CONFIG += console warn_on
CONFIG -= app_bundle
CONFIG -= qt


DESTDIR = build/lib
OBJECTS_DIR = build/obj


INCLUDEPATH += win32_api
INCLUDEPATH += linux_api
INCLUDEPATH += db_api
INCLUDEPATH += general
INCLUDEPATH += yserialport
INCLUDEPATH += ytimer
INCLUDEPATH += cpp_export
INCLUDEPATH += java_export/include



#win32平台下的处理
win32{
DEFINES +=  EV_WIN32                      #定义win32下的编译环境
DEF_FILE += win32_api/EV_protocol.def #只有win32才用的接口导出定位符
#QMAKE_CFLAGS +=--enable-stdcall-fixup
SOURCES += \
    yserialport/win_yocserialport.c \

HEADERS += \
    yserialport/win_yocserialport.h \
}

#unix平台下的处理
unix{
DEFINES += EV_UNIX  #定义linux下的编译环境

SOURCES += \
    yserialport/unix_yocserialport.c
HEADERS += \
    yserialport/unix_yocserialport.h
}

HEADERS += \
    ytimer/timer.h \
    general/LOGC.h \
    yserialport/yoc_serialbase.h \
    yserialport/yoc_serialport.h \
    general/ev_config.h \
    cpp_export/db_driver.h \
    db_api/db_serial.h \
    db_api/mdb_api.h \
    java_export/com_easivend_evprotocol_EVprotocol.h \
    db_api/db_json.h \
    db_api/bento_api.h \
    general/cJSON.h \
    db_api/column_api.h \
    db_api/lift_api.h

SOURCES += \
    ytimer/timer.c \
    general/LOGC.c \
    yserialport/yoc_serialbase.c \
    yserialport/yoc_serialport.c \
    general/ev_config.c \
    cpp_export/db_driver.c \
    db_api/db_serial.c \
    db_api/mdb_api.c \
    java_export/com_easivend_evprotocol_EVprotocol.c \
    db_api/db_json.c \
    db_api/bento_api.c \
    general/cJSON.c \
    db_api/column_api.c \
    db_api/lift_api.c

DISTFILES += \
    win32_api/EV_protocol.def





