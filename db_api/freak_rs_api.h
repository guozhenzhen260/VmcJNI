#ifndef FREAK_RS_API_H
#define FREAK_RS_API_H

#include "db_driver.h"



#define FRS_BUF_SIZE    128


int32 FRS_send(int32 fd,int32 port,char *data,int32 len);
int32 FRS_recv(int32 fd,int32 *port,char *data,int32 *rlen,int32 timeout);

int32 FRS_test(int32 fd);

#endif // FREAK_RS_API_H
