#ifndef LIFT_API_H
#define LIFT_API_H
#include "db_driver.h"

//VMC通知GCC复位初始化
#define VMC_RESET_REQ				(0x80)
//VMC通知GCC报告GCC状态
#define VMC_STATUS_REQ				(0x81)
//VMC通知GCC出货
#define VMC_VENDING_REQ				(0x82)
//VMC通知GCC报告出货情况
#define VMC_VENDINGRESULT_REQ		(0x83)
//VMC通知GCC开门
#define VMC_OPENDOOR_REQ			(0x84)
//VMC通知GCC关门
#define VMC_CLOSEDOOR_REQ			(0x85)
//VMC通知GCC打开照明灯
#define VMC_OPENLIGHT_REQ			(0x86)
//VMC通知GCC关闭照明灯
#define VMC_CLOSELIGHT_REQ			(0x87)
//VMC通知GCC报告配置参数
#define VMC_GETINFO_REQ				(0x88)

#define VMC_CHUCHOU_REQ				(0x8A)

#define GCC_STATUS_ACK				(0x01)
//GCC通知VMC出货
#define GCC_VENDING_ACK				(0x02)
//GCC通知VMC报告出货情况
#define GCC_VENDINGRESULT_ACK		(0x03)
//GCC通知VMC开门
#define GCC_OPENDOOR_ACK			(0x04)
//GCC通知VMC关门
#define GCC_CLOSEDOOR_ACK			(0x05)
//GCC通知VMC打开照明灯
#define GCC_OPENLIGHT_ACK			(0x06)
//GCC通知VMC关闭照明灯
#define GCC_CLOSELIGHT_ACK			(0x07)
//GCC通知VMC报告配置参数
#define GCC_GETINFO_ACK				(0x08)

#define GCC_CHUCHOU_ACK				(0x0A)

#define GCC_RESET_ACK               (0x00)








#endif // BENTO_API_H
