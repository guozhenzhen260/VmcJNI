#ifndef LIFT_API_H
#define LIFT_API_H
#include "db_driver.h"

//VMC֪ͨGCC��λ��ʼ��
#define VMC_RESET_REQ				(0x80)
//VMC֪ͨGCC����GCC״̬
#define VMC_STATUS_REQ				(0x81)
//VMC֪ͨGCC����
#define VMC_VENDING_REQ				(0x82)
//VMC֪ͨGCC����������
#define VMC_VENDINGRESULT_REQ		(0x83)
//VMC֪ͨGCC����
#define VMC_OPENDOOR_REQ			(0x84)
//VMC֪ͨGCC����
#define VMC_CLOSEDOOR_REQ			(0x85)
//VMC֪ͨGCC��������
#define VMC_OPENLIGHT_REQ			(0x86)
//VMC֪ͨGCC�ر�������
#define VMC_CLOSELIGHT_REQ			(0x87)
//VMC֪ͨGCC�������ò���
#define VMC_GETINFO_REQ				(0x88)

#define VMC_CHUCHOU_REQ				(0x8A)

#define GCC_STATUS_ACK				(0x01)
//GCC֪ͨVMC����
#define GCC_VENDING_ACK				(0x02)
//GCC֪ͨVMC����������
#define GCC_VENDINGRESULT_ACK		(0x03)
//GCC֪ͨVMC����
#define GCC_OPENDOOR_ACK			(0x04)
//GCC֪ͨVMC����
#define GCC_CLOSEDOOR_ACK			(0x05)
//GCC֪ͨVMC��������
#define GCC_OPENLIGHT_ACK			(0x06)
//GCC֪ͨVMC�ر�������
#define GCC_CLOSELIGHT_ACK			(0x07)
//GCC֪ͨVMC�������ò���
#define GCC_GETINFO_ACK				(0x08)

#define GCC_CHUCHOU_ACK				(0x0A)

#define GCC_RESET_ACK               (0x00)








#endif // BENTO_API_H
