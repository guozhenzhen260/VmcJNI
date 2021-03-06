/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
#include "db_driver.h"
#include "db_serial.h"
#include "db_json.h"
#include "yoc_serialport.h"
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include "ev_config.h"

/*********************************************************************************************************
**定义请求包类型
*********************************************************************************************************/
#define EV_NONE             0       //保留类型
#define EV_REGISTER         1       //串口注册
#define EV_RELEASE          2       //串口释放

//=====================快递柜类型==============================================================================


#define EV_BENTO_OPEN       11      //快递柜开门
#define EV_BENTO_CHECK      12      //快递柜查询
#define EV_BENTO_LIGHT      13      //快递柜照明
#define EV_BENTO_COOL       14      //快递柜制冷
#define EV_BENTO_HOT        15      //快递柜加热

//=====================普通柜类型==============================================================================
#define EV_COLUMN_OPEN      16      //主柜开门
#define EV_COLUMN_CHECK     17      //主柜查询
#define EV_COLUMN_TYPE      18      //主柜类型


//=====================MDB现金模组类型==============================================================================
#define EV_MDB_INIT         21      //MDB设备初始化
#define EV_MDB_ENABLE       22      //MDB设备使能
#define EV_MDB_HEART        23      //MDB设备心跳
#define EV_MDB_B_INFO       24      //MDB纸币器信息
#define EV_MDB_C_INFO       25      //MDB硬币器信息
#define EV_MDB_COST         26      //MDB设备扣款
#define EV_MDB_PAYBACK      27      //MDB设备退币
#define EV_MDB_PAYOUT       28      //MDB设备找币
#define EV_MDB_B_CON        29      //MDB纸币器配置
#define EV_MDB_C_CON        30      //MDB硬币器配置
#define EV_MDB_HP_PAYOUT    31      //hopper硬币器找币
#define EV_VBOX_TYPE        911
#define EV_CMD_FAIL         9998
#define EV_JSON_ERR         9999







/*
 * Class:     com_easivend_evprotocol_EVprotocol
 * Method:    EV_portRegister
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_easivend_evprotocol_EVprotocol_EVPortRegister
  (JNIEnv *env, jobject obj, jstring jportName)
{
    char *portName;
    int fd;
    jstring msg;
    char *text = NULL;
    cJSON *root,*entry;

    portName = (char *)(*env)->GetStringUTFChars(env,jportName, NULL);
    fd = EV_portRegister((char *)portName);


    root=cJSON_CreateObject();
    entry = cJSON_CreateObject();
    cJSON_AddItemToObject(root, JSON_HEAD,entry);
    cJSON_AddNumberToObject(entry,JSON_TYPE,EV_REGISTER);
    cJSON_AddStringToObject(entry,"port",portName);
    cJSON_AddNumberToObject(entry,"port_id",fd);

    text = cJSON_Print(root);
    cJSON_Delete(root);

    if(text != NULL){
        msg = (*env)->NewStringUTF(env,text);
        free(text);
    }
    else{
        msg = (*env)->NewStringUTF(env,"{\"EV_json\":{\"EV_type\":9999}}");
    }

    //释放java串口堆
    (*env)->ReleaseStringUTFChars(env,jportName,portName);


    return msg;
}



/*
 * Class:     com_easivend_evprotocol_EVprotocol
 * Method:    EVPortRelease
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_easivend_evprotocol_EVprotocol_EVPortRelease
  (JNIEnv *env, jclass cls, jstring jportName) 
{
	char *portName;
    jstring msg;
    char *text = NULL;
    cJSON *root,*entry;
	
	portName = (char *)(*env)->GetStringUTFChars(env,jportName, NULL);
    EV_portReleaseByName(portName);
	
root=cJSON_CreateObject();
entry = cJSON_CreateObject();
    cJSON_AddItemToObject(root, JSON_HEAD, entry);
    cJSON_AddNumberToObject(entry,JSON_TYPE,EV_RELEASE);

    cJSON_AddNumberToObject(entry,"result",1);
    text = cJSON_Print(root);
    cJSON_Delete(root);

    if(text != NULL){
        msg = (*env)->NewStringUTF(env,text);
        free(text);
    }
    else{
        msg = (*env)->NewStringUTF(env,"{\"EV_json\":{\"EV_type\":9999}}");

	}

	//释放java串口堆
    (*env)->ReleaseStringUTFChars(env,jportName,portName);
	
    return msg;
}


/*
 * Class:     com_easivend_evprotocol_EVprotocol
 * Method:    EVBentoOpen
 * Signature: (III)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_easivend_evprotocol_EVprotocol_EVBentoOpen
  (JNIEnv *env, jclass cls, jint fd, jint addr, jint box)
{
    jstring msg;
    char *text = NULL;
    cJSON *root,*entry;
    int res;
    ST_BT_OPEN_REQ req;
    ST_BT_OPEN_RPT rpt;

    req.fd = fd;
    req.addr = addr;
    req.no = box;
    res = EV_bentoOpen(&req,&rpt);


    root=cJSON_CreateObject();
    entry = cJSON_CreateObject();
    cJSON_AddItemToObject(root, JSON_HEAD, entry );
    cJSON_AddNumberToObject(entry,JSON_TYPE,EV_BENTO_OPEN);

    cJSON_AddNumberToObject(entry,"port_id",rpt.fd);
    cJSON_AddNumberToObject(entry,"addr",rpt.addr);
    cJSON_AddNumberToObject(entry,"box",rpt.no);


    if(res == 1){
        cJSON_AddNumberToObject(entry,"is_success",1);
        cJSON_AddNumberToObject(entry,"result",rpt.result);
    }
    else{
        cJSON_AddNumberToObject(entry,"is_success",0);
        cJSON_AddNumberToObject(entry,"result",0);
    }


    text = cJSON_Print(root);
    cJSON_Delete(root);

    if(text != NULL){
        msg = (*env)->NewStringUTF(env,text);
        free(text);
    }
    else{
        msg = (*env)->NewStringUTF(env,"{\"EV_json\":{\"EV_type\":9999}}");
    }

    return msg;
}



/*
 * Class:     com_easivend_evprotocol_EVprotocol
 * Method:    EVBentoCheck
 * Signature: (II)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_easivend_evprotocol_EVprotocol_EVBentoCheck
  (JNIEnv *env, jclass cls, jint fd, jint addr)
{
    ST_BT_CHECK_REQ req;
    ST_BT_CHECK_RPT rpt;
    char ID[64] = {0};
    int res,i;
    cJSON *root,*entry,*col,*arr;
    char *text = NULL;
    jstring msg;
    req.fd = fd;
    req.addr = addr;
    res = EV_bentoCheck(&req,&rpt);

    root=cJSON_CreateObject();
    entry = cJSON_CreateObject();
    cJSON_AddItemToObject(root, JSON_HEAD, entry );
    cJSON_AddNumberToObject(entry,JSON_TYPE,EV_BENTO_CHECK);


    cJSON_AddNumberToObject(entry,"port_id",rpt.fd);
    cJSON_AddNumberToObject(entry,"addr",rpt.addr);
    if(res == 1){
        cJSON_AddNumberToObject(entry,"is_success",1);
        for(i = 0;i < rpt.id_len ;i++){
            sprintf(&ID[i*2],"%02x",rpt.id[i]);
        }
        cJSON_AddStringToObject(entry,"ID",ID);
        cJSON_AddNumberToObject(entry,"cool",rpt.iscool);
        cJSON_AddNumberToObject(entry,"hot",rpt.ishot);
        cJSON_AddNumberToObject(entry,"light",rpt.islight);
        cJSON_AddNumberToObject(entry,"sum",rpt.sum);

        cJSON_AddItemToObject(entry,"column",arr = cJSON_CreateArray());
        for(i = 0;i < rpt.sum;i++){  //遍历货道
            cJSON_AddItemToArray(arr,col = cJSON_CreateObject());
            cJSON_AddNumberToObject(col,"no",i + 1);
            cJSON_AddNumberToObject(col,"state",1);
        }
    }
    else{
        cJSON_AddNumberToObject(entry,"is_success",0);
    }

    text = cJSON_Print(root);
    cJSON_Delete(root);
    if(text != NULL){
        msg = (*env)->NewStringUTF(env,text);
        free(text);
    }
    else{
        msg = (*env)->NewStringUTF(env,"{\"EV_json\":{\"EV_type\":9999}}");
    }

    return msg;

}



/*
 * Class:     com_easivend_evprotocol_EVprotocol
 * Method:    EVBentoLight
 * Signature: (III)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_easivend_evprotocol_EVprotocol_EVBentoLight
  (JNIEnv *env, jclass cls, jint fd, jint addr, jint opt)
{
    ST_BT_LIGHT_REQ req;
    ST_BT_LIGHT_RPT rpt;
    int res;
    jstring msg;
    char *text = NULL;
    cJSON *root,*entry;

    req.fd = fd;
    req.addr =addr;
    req.opt = opt;

    res = EV_bentoLight(&req,&rpt);
    root=cJSON_CreateObject();
    entry = cJSON_CreateObject();
    cJSON_AddItemToObject(root, JSON_HEAD, entry);
    cJSON_AddNumberToObject(entry,JSON_TYPE,EV_BENTO_LIGHT);

    cJSON_AddNumberToObject(entry,"port_id",rpt.fd);
    cJSON_AddNumberToObject(entry,"addr",rpt.addr);
    cJSON_AddNumberToObject(entry,"opt",rpt.opt);

    if(res == 1){
        cJSON_AddNumberToObject(entry,"is_success",1);
        cJSON_AddNumberToObject(entry,"result",rpt.res);
    }
    else{
        cJSON_AddNumberToObject(entry,"is_success",0);
        cJSON_AddNumberToObject(entry,"result",0);
    }

    text = cJSON_Print(root);
    cJSON_Delete(root);
    if(text != NULL){
        msg = (*env)->NewStringUTF(env,text);
        free(text);
    }
    else{
        msg = (*env)->NewStringUTF(env,"{\"EV_json\":{\"EV_type\":9999}}");
    }

    return msg;


}

/*
 * Class:     com_easivend_evprotocol_EVprotocol
 * Method:    EVBentoCool
 * Signature: (III)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_easivend_evprotocol_EVprotocol_EVBentoCool
  (JNIEnv *env, jclass cls, jint fd, jint addr, jint opt)
{
    ST_BT_COOL_REQ req;
    ST_BT_COOL_RPT rpt;
    int res;
    jstring msg;
    char *text = NULL;
    cJSON *root,*entry;

    req.fd = fd;
    req.addr =addr;
    req.opt = opt;

    res = EV_bentoCool(&req,&rpt);
    root=cJSON_CreateObject();
    entry = cJSON_CreateObject();
    cJSON_AddItemToObject(root, JSON_HEAD, entry);
    cJSON_AddNumberToObject(entry,JSON_TYPE,EV_BENTO_COOL);

    cJSON_AddNumberToObject(entry,"port_id",rpt.fd);
    cJSON_AddNumberToObject(entry,"addr",rpt.addr);
    cJSON_AddNumberToObject(entry,"opt",rpt.opt);

    if(res == 1){
        cJSON_AddNumberToObject(entry,"is_success",1);
        cJSON_AddNumberToObject(entry,"result",rpt.res);
    }
    else{
        cJSON_AddNumberToObject(entry,"is_success",0);
        cJSON_AddNumberToObject(entry,"result",0);
    }

    text = cJSON_Print(root);
    cJSON_Delete(root);
    if(text != NULL){
        msg = (*env)->NewStringUTF(env,text);
        free(text);
    }
    else{
        msg = (*env)->NewStringUTF(env,"{\"EV_json\":{\"EV_type\":9999}}");
    }

    return msg;


}

/*
 * Class:     com_easivend_evprotocol_EVprotocol
 * Method:    EVBentoHot
 * Signature: (III)Ljava/lang/String; 
 */
JNIEXPORT jstring JNICALL Java_com_easivend_evprotocol_EVprotocol_EVBentoHot
  (JNIEnv *env, jclass cls, jint fd, jint addr, jint opt)
{
    ST_BT_HOT_REQ req;
    ST_BT_HOT_RPT rpt;
    int res;
    jstring msg;
    char *text = NULL;
    cJSON *root,*entry;

    req.fd = fd;
    req.addr =addr;
    req.opt = opt;

    res = EV_bentoHot(&req,&rpt);
    root=cJSON_CreateObject();
    entry = cJSON_CreateObject();
    cJSON_AddItemToObject(root, JSON_HEAD, entry);
    cJSON_AddNumberToObject(entry,JSON_TYPE,EV_BENTO_HOT);

    cJSON_AddNumberToObject(entry,"port_id",rpt.fd);
    cJSON_AddNumberToObject(entry,"addr",rpt.addr);
    cJSON_AddNumberToObject(entry,"opt",rpt.opt);

    if(res == 1){
        cJSON_AddNumberToObject(entry,"is_success",1);
        cJSON_AddNumberToObject(entry,"result",rpt.res);
    }
    else{
        cJSON_AddNumberToObject(entry,"is_success",0);
        cJSON_AddNumberToObject(entry,"result",0);
    }

    text = cJSON_Print(root);
    cJSON_Delete(root);
    if(text != NULL){
        msg = (*env)->NewStringUTF(env,text);
        free(text);
    }
    else{
        msg = (*env)->NewStringUTF(env,"{\"EV_json\":{\"EV_type\":9999}}");
    }

    return msg;


}


/*********************************************************************************************************
** Function name	:		trade
** Descriptions		:		普通柜出货接口  [异步]
** input parameters	:       fd:串口编号, columntype:货道类型1弹簧，3升降台+传送带，4升降台+弹簧
*                          ,addr:柜子地址 01-16,box:开门的格子号 1-88
** output parameters:		无
** Returned value	:		1：发送成功  0：发送失败
*	返回json包     例如： EV_JSON={"EV_json":{"EV_type":16,"port_id":2,"addr":1,"box":34,"is_success":1,"result":0}}
*							"EV_type"= EV_COLUMN_OPEN = 16; 表弹簧出货结果回应包类型
*							"port_id":原样返回,
*							"is_success":表示指令是否发送成功,1:发送成功。 0:发送失败（通信超时）
*							"result": 	表示处理结果	
*********************************************************************************************************/
JNIEXPORT jstring JNICALL Java_com_easivend_evprotocol_EVprotocol_EVtrade
(JNIEnv *env, jclass cls, jint fd, jint columntype,jint addr, jint box, jint goc)
{
    jstring msg;
    char *text = NULL;
    cJSON *root,*entry;
    int res;
    ST_COL_OPEN_REQ req;
    ST_COL_OPEN_RPT rpt;

    req.fd = fd;
    req.addr = addr;
    req.no = box;
	req.goc= goc;

	switch(columntype){
		case 1:
			res = EV_columnOpen(&req,&rpt);
			break;
        case 3:
			res = EV_liftVendout(&req,&rpt);
			break;
        case 4:
            res = EV_column363xOpen(&req, &rpt);
            break;
		default:res = 0;break;
	}
    
    root=cJSON_CreateObject();
    entry = cJSON_CreateObject();
    cJSON_AddItemToObject(root, JSON_HEAD, entry );
    cJSON_AddNumberToObject(entry,JSON_TYPE,EV_COLUMN_OPEN);
    cJSON_AddNumberToObject(entry,"port_id",rpt.fd);
    cJSON_AddNumberToObject(entry,"addr",rpt.addr);
    cJSON_AddNumberToObject(entry,"box",rpt.no);
    if(res == 1)
	{
        cJSON_AddNumberToObject(entry,"is_success",1);
        cJSON_AddNumberToObject(entry,"result",rpt.result);
    }
    else
	{
        cJSON_AddNumberToObject(entry,"is_success",0);
        cJSON_AddNumberToObject(entry,"result",0);
    }
	
    text = cJSON_Print(root);
    cJSON_Delete(root);
    if(text != NULL){
        msg = (*env)->NewStringUTF(env,text);
        free(text);
    }
    else{
        msg = (*env)->NewStringUTF(env,"{\"EV_json\":{\"EV_type\":9999}}");
    }   

    return msg;
}





/*
 * Class:     com_easivend_evprotocol_EVprotocol
 * Method:    EVmdbInit
 * Signature: (III)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_easivend_evprotocol_EVprotocol_EVmdbInit
  (JNIEnv *env, jclass cls, jint fd, jint bill, jint coin)
{
    ST_MDB_INIT_REQ req;
    ST_MDB_INIT_RPT rpt;
    int res;
    jstring msg;
    char *text= NULL;
    cJSON *root,*entry;


    req.fd =    fd;
    req.bill =  bill;
    req.coin =  coin;
    res = EV_mdbInit(&req,&rpt);
    root=cJSON_CreateObject();
    entry = cJSON_CreateObject();
    cJSON_AddItemToObject(root, JSON_HEAD, entry );
    cJSON_AddNumberToObject(entry,JSON_TYPE,EV_MDB_INIT);

    cJSON_AddNumberToObject(entry,"port_id",rpt.fd);
    cJSON_AddNumberToObject(entry,"bill",rpt.bill);
    cJSON_AddNumberToObject(entry,"coin",rpt.coin);

    if(res == 1){
        cJSON_AddNumberToObject(entry,"is_success",rpt.res);
        cJSON_AddNumberToObject(entry,"bill_result",rpt.billResult);
        cJSON_AddNumberToObject(entry,"coin_result",rpt.coinResult);
    }
    else{
        cJSON_AddNumberToObject(entry,"is_success",0);
    }

    text = cJSON_Print(root);
    cJSON_Delete(root);
    if(text != NULL){
        msg = (*env)->NewStringUTF(env,text);
        free(text);
    }
    else{
        msg = (*env)->NewStringUTF(env,"{\"EV_json\":{\"EV_type\":9999}}");
    }

    return msg;

}




/*
 * Class:     com_easivend_evprotocol_EVprotocol
 * Method:    EVmdbEnable
 * Signature: (IIII)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_easivend_evprotocol_EVprotocol_EVmdbEnable
  (JNIEnv *env, jclass cls, jint fd, jint bill, jint coin, jint opt)
{
    ST_MDB_ENABLE_REQ req;
    ST_MDB_ENABLE_RPT rpt;
    int res;
    jstring msg;
    char *text = NULL;
    cJSON *root,*entry;


    req.fd =    fd;
    req.bill =  bill;
    req.coin =  coin;
    req.opt =   opt;
    res = EV_mdbEnable(&req,&rpt);
    root=cJSON_CreateObject();
    entry = cJSON_CreateObject();
    cJSON_AddItemToObject(root, JSON_HEAD, entry);
    cJSON_AddNumberToObject(entry,JSON_TYPE,EV_MDB_ENABLE);


    cJSON_AddNumberToObject(entry,"port_id",rpt.fd);
    cJSON_AddNumberToObject(entry,"bill",rpt.bill);
    cJSON_AddNumberToObject(entry,"coin",rpt.coin);
    cJSON_AddNumberToObject(entry,"opt",rpt.opt);


    if(res == 1){
        cJSON_AddNumberToObject(entry,"is_success",rpt.res);
        cJSON_AddNumberToObject(entry,"bill_result",rpt.billResult);
        cJSON_AddNumberToObject(entry,"coin_result",rpt.coinResult);
    }
    else{
        cJSON_AddNumberToObject(entry,"is_success",0);
    }

    text = cJSON_Print(root);
    cJSON_Delete(root);
    if(text != NULL){
        msg = (*env)->NewStringUTF(env,text);
        free(text);
    }
    else{
        msg = (*env)->NewStringUTF(env,"{\"EV_json\":{\"EV_type\":9999}}");
    }

    return msg;
}




/*
 * Class:     com_easivend_evprotocol_EVprotocol
 * Method:    EVmdbHeart
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_easivend_evprotocol_EVprotocol_EVmdbHeart
  (JNIEnv *env, jclass cls, jint fd)
{
    ST_MDB_HEART_REQ req;
    ST_MDB_HEART_RPT rpt;
    int res;
    jstring msg;
    char *text = NULL;
    cJSON *root,*entry,*hopper;


    req.fd =    fd;
    res = EV_mdbHeartCheck(&req,&rpt);
    root=cJSON_CreateObject();
    entry = cJSON_CreateObject();
    cJSON_AddItemToObject(root, JSON_HEAD, entry);
    cJSON_AddNumberToObject(entry,JSON_TYPE,EV_MDB_HEART);

    cJSON_AddNumberToObject(entry,"port_id",rpt.fd);

    if(res == 1){
        cJSON_AddNumberToObject(entry,"is_success",rpt.res);
        cJSON_AddNumberToObject(entry,"bill_enable",rpt.billEnable);
        cJSON_AddNumberToObject(entry,"bill_payback",rpt.billPayback);
        cJSON_AddNumberToObject(entry,"bill_err",rpt.billErr);
        cJSON_AddNumberToObject(entry,"bill_recv",rpt.billAmount);
        cJSON_AddNumberToObject(entry,"bill_remain",rpt.billRemain);

        cJSON_AddNumberToObject(entry,"coin_enable",rpt.coinEnable);
        cJSON_AddNumberToObject(entry,"coin_payback",rpt.coinPayback);


        cJSON_AddNumberToObject(entry,"coin_err",rpt.coinErr);
        if(rpt.isHopper == 1){
            hopper = cJSON_CreateObject();
            cJSON_AddItemToObject(entry,"hopper",hopper);
            cJSON_AddNumberToObject(hopper,"hopper1",rpt.hopper[0]);
            cJSON_AddNumberToObject(hopper,"hopper2",rpt.hopper[1]);
            cJSON_AddNumberToObject(hopper,"hopper3",rpt.hopper[2]);
            cJSON_AddNumberToObject(hopper,"hopper4",rpt.hopper[3]);
            cJSON_AddNumberToObject(hopper,"hopper5",rpt.hopper[4]);
            cJSON_AddNumberToObject(hopper,"hopper6",rpt.hopper[5]);
            cJSON_AddNumberToObject(hopper,"hopper7",rpt.hopper[6]);
            cJSON_AddNumberToObject(hopper,"hopper8",rpt.hopper[7]);
        }

        cJSON_AddNumberToObject(entry,"coin_recv",rpt.coinAmount);
        cJSON_AddNumberToObject(entry,"coin_remain",rpt.coinRemain);

    }
    else{
        cJSON_AddNumberToObject(entry,"is_success",0);
    }

    text = cJSON_Print(root);
    cJSON_Delete(root);
    if(text != NULL){
        msg = (*env)->NewStringUTF(env,text);
        free(text);
    }
    else{
        msg = (*env)->NewStringUTF(env,"{\"EV_json\":{\"EV_type\":9999}}");
    }

    return msg;
}



/*
 * Class:     com_easivend_evprotocol_EVprotocol
 * Method:    EVmdbBillInfoCheck
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_easivend_evprotocol_EVprotocol_EVmdbBillInfoCheck
  (JNIEnv *env, jclass cls, jint fd)
{
    ST_MDB_BILL_INFO_REQ req;
    ST_MDB_BILL_INFO_RPT rpt;
    int res,i;
    jstring msg;
    char *text = NULL,buf[256];
    cJSON *root,*entry,*arr,*channel;


    req.fd =    fd;
    res = EV_mdbBillInfoCheck(&req,&rpt);
    root=cJSON_CreateObject();
    entry = cJSON_CreateObject();;
    cJSON_AddItemToObject(root, JSON_HEAD, entry);
    cJSON_AddNumberToObject(entry,JSON_TYPE,EV_MDB_B_INFO);

    cJSON_AddNumberToObject(entry,"port_id",rpt.fd);

    if(res == 1){
        cJSON_AddNumberToObject(entry,"is_success",rpt.res);
        cJSON_AddNumberToObject(entry,"acceptor",rpt.acceptor);
        cJSON_AddNumberToObject(entry,"dispenser",rpt.dispenser);


        cJSON_AddStringToObject(entry,"code",rpt.code);
        cJSON_AddStringToObject(entry,"sn",rpt.sn);
        cJSON_AddStringToObject(entry,"model",rpt.model);

        memset(buf,0,sizeof(buf));
        for(i = 0;i < 2;i++){
           sprintf(&buf[i*2],"%02x",rpt.ver[i]);
        }
        cJSON_AddStringToObject(entry,"ver",buf);
        cJSON_AddNumberToObject(entry,"capacity",rpt.capacity);

        cJSON_AddItemToObject(entry,"ch_r",arr = cJSON_CreateArray());

        for(i = 0;i < 16;i++){
            cJSON_AddItemToArray(arr,channel = cJSON_CreateObject());
            cJSON_AddNumberToObject(channel,"ch",i + 1);
            cJSON_AddNumberToObject(channel,"value",rpt.ch_r[i]);
        }   
        cJSON_AddItemToObject(entry,"ch_d",arr = cJSON_CreateArray());

        for(i = 0;i < 16;i++){
            cJSON_AddItemToArray(arr,channel = cJSON_CreateObject());
            cJSON_AddNumberToObject(channel,"ch",i + 1);
            cJSON_AddNumberToObject(channel,"value",rpt.ch_d[i]);

        }

    }
    else{
        cJSON_AddNumberToObject(entry,"is_success",0);
    }

    text = cJSON_Print(root);
    cJSON_Delete(root);

    if(text != NULL){
        msg = (*env)->NewStringUTF(env,text);
        free(text);
    }
    else{
        msg = (*env)->NewStringUTF(env,"{\"EV_json\":{\"EV_type\":9999}}");
    }

    return msg;
}


/*
 * Class:     com_easivend_evprotocol_EVprotocol
 * Method:    EVmdbCoinInfoCheck
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_easivend_evprotocol_EVprotocol_EVmdbCoinInfoCheck
  (JNIEnv *env, jclass cls, jint fd)
{
    ST_MDB_COIN_INFO_REQ req;
    ST_MDB_COIN_INFO_RPT rpt;
    int res,i;
    jstring msg;
    char *text = NULL,buf[256];
    cJSON *root,*entry,*arr,*channel;


    req.fd =    fd;
    res = EV_mdbCoinInfoCheck(&req,&rpt);

    root=cJSON_CreateObject();
    entry = cJSON_CreateObject();
    cJSON_AddItemToObject(root, JSON_HEAD, entry );
    cJSON_AddNumberToObject(entry,JSON_TYPE,EV_MDB_C_INFO);


    cJSON_AddNumberToObject(entry,"port_id",rpt.fd);

    if(res == 1){
        cJSON_AddNumberToObject(entry,"is_success",rpt.res);
        cJSON_AddNumberToObject(entry,"acceptor",rpt.acceptor);
        cJSON_AddNumberToObject(entry,"dispenser",rpt.dispenser);


        cJSON_AddStringToObject(entry,"code",rpt.code);
        cJSON_AddStringToObject(entry,"sn",rpt.sn);
        cJSON_AddStringToObject(entry,"model",rpt.model);

        memset(buf,0,sizeof(buf));
        for(i = 0;i < 2;i++){
           sprintf(&buf[i*2],"%02x",rpt.ver[i]);
        }
        cJSON_AddStringToObject(entry,"ver",buf);
        cJSON_AddNumberToObject(entry,"capacity",rpt.capacity);



        cJSON_AddItemToObject(entry,"ch_r",arr = cJSON_CreateArray());

        for(i = 0;i < 16;i++){
            cJSON_AddItemToArray(arr,channel = cJSON_CreateObject());
            cJSON_AddNumberToObject(channel,"ch",i + 1);
            cJSON_AddNumberToObject(channel,"value",rpt.ch_r[i]);
        }
        cJSON_AddItemToObject(entry,"ch_d",arr = cJSON_CreateArray());

        for(i = 0;i < 16;i++){
            cJSON_AddItemToArray(arr,channel = cJSON_CreateObject());
            cJSON_AddNumberToObject(channel,"ch",i + 1);
            cJSON_AddNumberToObject(channel,"value",rpt.ch_d[i]);

        }

    }
    else{
        cJSON_AddNumberToObject(entry,"is_success",0);
    }

    text = cJSON_Print(root);
    cJSON_Delete(root);
    if(text != NULL){
        msg = (*env)->NewStringUTF(env,text);
        free(text);
    }
    else{
        msg = (*env)->NewStringUTF(env,"{\"EV_json\":{\"EV_type\":9999}}");
    }

    return msg;
}



/*
 * Class:     com_easivend_evprotocol_EVprotocol
 * Method:    EVmdbCost
 * Signature: (II)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_easivend_evprotocol_EVprotocol_EVmdbCost
  (JNIEnv *env, jclass cls, jint fd, jint cost)
{
    ST_MDB_COST_REQ req;
    ST_MDB_COST_RPT rpt;
    int res;
    jstring msg;
    char *text = NULL;
    cJSON *root,*entry;


    req.fd =    fd;
    req.cost = cost;
    res = EV_mdbCost(&req,&rpt);
    root=cJSON_CreateObject();
    entry = cJSON_CreateObject();
    cJSON_AddItemToObject(root, JSON_HEAD, entry);
    cJSON_AddNumberToObject(entry,JSON_TYPE,EV_MDB_COST);


    cJSON_AddNumberToObject(entry,"port_id",rpt.fd);
    cJSON_AddNumberToObject(entry,"cost",rpt.cost);
    if(res == 1){
        cJSON_AddNumberToObject(entry,"is_success",1);
        cJSON_AddNumberToObject(entry,"result",rpt.res);
        cJSON_AddNumberToObject(entry,"bill_recv",rpt.billAmount);
        cJSON_AddNumberToObject(entry,"coin_recv",rpt.coinAmount);
    }
    else{
        cJSON_AddNumberToObject(entry,"is_success",0);
        cJSON_AddNumberToObject(entry,"result",0);
    }

    text = cJSON_Print(root);
    cJSON_Delete(root);
    if(text != NULL){
        msg = (*env)->NewStringUTF(env,text);
        free(text);
    }
    else{
        msg = (*env)->NewStringUTF(env,"{\"EV_json\":{\"EV_type\":9999}}");
    }

    return msg;
}


/*
 * Class:     com_easivend_evprotocol_EVprotocol
 * Method:    EVmdbPayback
 * Signature: (III)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_easivend_evprotocol_EVprotocol_EVmdbPayback
  (JNIEnv *env, jclass cls, jint fd, jint bill, jint coin)
{

    ST_MDB_PAYBACK_REQ req;
    ST_MDB_PAYBACK_RPT rpt;
    int res;
    jstring msg;
    char *text = NULL;
    cJSON *root,*entry;


    req.fd =    fd;
    req.bill = bill;
    req.coin = coin;
    res = EV_mdbPayBack(&req,&rpt);
    root=cJSON_CreateObject();
    entry = cJSON_CreateObject();
    cJSON_AddItemToObject(root, JSON_HEAD, entry);
    cJSON_AddNumberToObject(entry,JSON_TYPE,EV_MDB_PAYBACK);

    cJSON_AddNumberToObject(entry,"port_id",rpt.fd);
    cJSON_AddNumberToObject(entry,"bill",rpt.bill);
    cJSON_AddNumberToObject(entry,"coin",rpt.coin);
    if(res == 1){
        cJSON_AddNumberToObject(entry,"is_success",1);
        cJSON_AddNumberToObject(entry,"result",rpt.res);
        cJSON_AddNumberToObject(entry,"bill_changed",rpt.billChanged);
        cJSON_AddNumberToObject(entry,"coin_changed",rpt.coinChanged);
    }
    else{
        cJSON_AddNumberToObject(entry,"is_success",0);
        cJSON_AddNumberToObject(entry,"result",0);
    }

    text = cJSON_Print(root);
    cJSON_Delete(root);
    if(text != NULL){
        msg = (*env)->NewStringUTF(env,text);
        free(text);
    }
    else{
        msg = (*env)->NewStringUTF(env,"{\"EV_json\":{\"EV_type\":9999}}");
    }

    return msg;

}


/*
 * Class:     com_easivend_evprotocol_EVprotocol
 * Method:    EVmdbHopperPayout
 * Signature: (III)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_easivend_evprotocol_EVprotocol_EVmdbHopperPayout
  (JNIEnv *env, jclass cls, jint fd, jint no, jint nums)
{
    ST_MDB_HP_PAYOUT_REQ req;
    ST_MDB_HP_PAYOUT_RPT rpt;
    int res;
    jstring msg;
    char *text = NULL;
    cJSON *root,*entry;

    req.fd =    fd;
    req.no = no;
    req.nums = nums;

    res = EV_mdbHpPayout(&req,&rpt);

    root=cJSON_CreateObject();
    entry = cJSON_CreateObject();
    cJSON_AddItemToObject(root, JSON_HEAD, entry);
    cJSON_AddNumberToObject(entry,JSON_TYPE,EV_MDB_HP_PAYOUT);
    cJSON_AddNumberToObject(entry,"port_id",rpt.fd);
    cJSON_AddNumberToObject(entry,"no",rpt.no);
    cJSON_AddNumberToObject(entry,"nums",rpt.nums);

    if(res == 1){
        cJSON_AddNumberToObject(entry,"is_success",1);
        cJSON_AddNumberToObject(entry,"result",rpt.res);
        cJSON_AddNumberToObject(entry,"changed",rpt.changed);
    }
    else{
        cJSON_AddNumberToObject(entry,"is_success",0);
        cJSON_AddNumberToObject(entry,"result",0);
    }

    text = cJSON_Print(root);
    cJSON_Delete(root);
    if(text != NULL){
        msg = (*env)->NewStringUTF(env,text);
        free(text);

    }
    else{
        msg = (*env)->NewStringUTF(env,"{\"EV_json\":{\"EV_type\":9999}}");
    }

    return msg;

}

/*
 * Class:     com_easivend_evprotocol_EVprotocol
 * Method:    EVmdbPayout
 * Signature: (IIIII)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_easivend_evprotocol_EVprotocol_EVmdbPayout
  (JNIEnv *env, jclass cls, jint fd, jint bill, jint coin, jint billPay, jint coinPay)
{
    ST_MDB_PAYOUT_REQ req;
    ST_MDB_PAYOUT_RPT rpt;
    int res;
    jstring msg;
    char *text = NULL;
    cJSON *root,*entry;


    req.fd =    fd;
    req.bill = bill;
    req.coin = coin;
    req.billPayout = billPay;
    req.coinPayout = coinPay;
    res = EV_mdbPayout(&req,&rpt);

    root=cJSON_CreateObject();
    entry = cJSON_CreateObject();
    cJSON_AddItemToObject(root, JSON_HEAD, entry);
    cJSON_AddNumberToObject(entry,JSON_TYPE,EV_MDB_PAYOUT);

    cJSON_AddNumberToObject(entry,"port_id",rpt.fd);
    cJSON_AddNumberToObject(entry,"bill",rpt.bill);
    cJSON_AddNumberToObject(entry,"coin",rpt.coin);
    cJSON_AddNumberToObject(entry,"billPayout",rpt.billPayout);
    cJSON_AddNumberToObject(entry,"coinPayout",rpt.coinPayout);
    if(res == 1){
        cJSON_AddNumberToObject(entry,"is_success",1);
        cJSON_AddNumberToObject(entry,"result",rpt.res);
        cJSON_AddNumberToObject(entry,"bill_changed",rpt.billChanged);
        cJSON_AddNumberToObject(entry,"coin_changed",rpt.coinChanged);
    }
    else{
        cJSON_AddNumberToObject(entry,"is_success",0);
        cJSON_AddNumberToObject(entry,"result",0);
    }

    text = cJSON_Print(root);
    cJSON_Delete(root);
    if(text != NULL){
        msg = (*env)->NewStringUTF(env,text);
        free(text);

    }
    else{
        msg = (*env)->NewStringUTF(env,"{\"EV_json\":{\"EV_type\":9999}}");
    }

    return msg;
}



/*
 * Class:     com_easivend_evprotocol_EVprotocol
 * Method:    EVmdbBillConfig
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_easivend_evprotocol_EVprotocol_EVmdbBillConfig
  (JNIEnv *env , jclass cls, jstring reqStr)
{
    ST_MDB_BILL_CON_REQ req;
    ST_MDB_BILL_CON_RPT rpt;

    int res,i;
    jstring msg;
    char *text = NULL,*str;
    cJSON *root,*entry,*t1,*t2,*t3,*t4;
    char *err = "{\"EV_json\":{\"EV_type\":9998}}";

    str = (char *)(*env)->GetStringUTFChars(env,reqStr, NULL);
    root=cJSON_Parse((const char *)str);
    (*env)->ReleaseStringUTFChars(env,reqStr,str);


    if (!root) {
        return  (*env)->NewStringUTF(env,err);
    }

    t1 = cJSON_GetObjectItem(root,"EV_json");
    if(t1 == NULL){
        return  (*env)->NewStringUTF(env,err);
    }

    t2 = cJSON_GetObjectItem(t1,"EV_type");
    if(t2 == NULL || t2->type != cJSON_Number || t2->valueint != EV_MDB_B_CON){
        return  (*env)->NewStringUTF(env,err);
    }

    t2 = cJSON_GetObjectItem(t1,"port_id");
    if(t2 == NULL || t2->type != cJSON_Number){
        return  (*env)->NewStringUTF(env,err);
    }
    else{
        req.fd = t2->valueint;
    }

    t2 = cJSON_GetObjectItem(t1,"acceptor");
    if(t2 == NULL || t2->type != cJSON_Number){
        return  (*env)->NewStringUTF(env,err);
    }
    else{
        req.acceptor = t2->valueint;
    }

    t2 = cJSON_GetObjectItem(t1,"dispenser");
    if(t2 == NULL || t2->type != cJSON_Number){
        return  (*env)->NewStringUTF(env,err);
    }
    else{
        req.dispenser = t2->valueint;
    }

    t2 = cJSON_GetObjectItem(t1,"ch_r");
    if(t2 != NULL && t2->type == cJSON_Array){
        for(i = 0;i < 16;i++){
            t3 = cJSON_GetArrayItem(t2,i);
            if(t3 != NULL && t3->type == cJSON_Object){
                t4 = cJSON_GetObjectItem(t3,"value");
                if(t4 != NULL && t4->type == cJSON_Number){
                        req.ch_r[i] = t4->valueint;
                }
            }
        }
    }

    t2 = cJSON_GetObjectItem(t1,"ch_d");
    if(t2 != NULL && t2->type == cJSON_Array){
        for(i = 0;i < 16;i++){
            t3 = cJSON_GetArrayItem(t2,i);
            if(t3 != NULL && t3->type == cJSON_Object){
                t4 = cJSON_GetObjectItem(t3,"value");
                if(t4 != NULL && t4->type == cJSON_Number){
                        req.ch_d[i] = t4->valueint;
                }
            }
        }
    }

    cJSON_Delete(root); //释放root

    res = EV_mdbBillConfig(&req,&rpt);

    root=cJSON_CreateObject();
    entry = cJSON_CreateObject();
    cJSON_AddItemToObject(root, JSON_HEAD, entry);
    cJSON_AddNumberToObject(entry,JSON_TYPE,EV_MDB_B_CON);

    cJSON_AddNumberToObject(entry,"port_id",rpt.fd);
    cJSON_AddNumberToObject(entry,"acceptor",rpt.acceptor);
    cJSON_AddNumberToObject(entry,"dispenser",rpt.dispenser);
    if(res == 1){
        cJSON_AddNumberToObject(entry,"is_success",rpt.com_ok);
        cJSON_AddNumberToObject(entry,"result",rpt.res);
    }
    else{
        cJSON_AddNumberToObject(entry,"is_success",0);
        cJSON_AddNumberToObject(entry,"result",0);
    }

    text = cJSON_Print(root);
    cJSON_Delete(root);
    if(text != NULL){
        msg = (*env)->NewStringUTF(env,text);
        free(text);

    }
    else{
        msg = (*env)->NewStringUTF(env,"{\"EV_json\":{\"EV_type\":9999}}");
    }
    return msg;

}



/*
 * Class:     com_easivend_evprotocol_EVprotocol
 * Method:    EVmdbCoinConfig
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_easivend_evprotocol_EVprotocol_EVmdbCoinConfig
  (JNIEnv *env, jclass cls, jstring reqStr)
{
    ST_MDB_COIN_CON_REQ req;
    ST_MDB_COIN_CON_RPT rpt;

    int res,i;
    jstring msg;
    char *text = NULL,*str;
    cJSON *root,*entry,*t1,*t2,*t3,*t4;
    char *err = "{\"EV_json\":{\"EV_type\":9998}}";

    str = (char *)(*env)->GetStringUTFChars(env,reqStr, NULL);
    root=cJSON_Parse((const char *)str);
    (*env)->ReleaseStringUTFChars(env,reqStr,str);


    if (!root) {
        return  (*env)->NewStringUTF(env,err);
    }

    t1 = cJSON_GetObjectItem(root,"EV_json");
    if(t1 == NULL){
        return  (*env)->NewStringUTF(env,err);
    }

    t2 = cJSON_GetObjectItem(t1,"EV_type");
    if(t2 == NULL || t2->type != cJSON_Number || t2->valueint != EV_MDB_C_CON){
        return  (*env)->NewStringUTF(env,err);
    }

    t2 = cJSON_GetObjectItem(t1,"port_id");
    if(t2 == NULL || t2->type != cJSON_Number){
        return  (*env)->NewStringUTF(env,err);
    }
    else{
        req.fd = t2->valueint;
    }

    t2 = cJSON_GetObjectItem(t1,"acceptor");
    if(t2 == NULL || t2->type != cJSON_Number){
        return  (*env)->NewStringUTF(env,err);
    }
    else{
        req.acceptor = t2->valueint;
    }

    t2 = cJSON_GetObjectItem(t1,"dispenser");
    if(t2 == NULL || t2->type != cJSON_Number){
        return  (*env)->NewStringUTF(env,err);
    }
    else{
        req.dispenser = t2->valueint;
    }

    t2 = cJSON_GetObjectItem(t1,"hight_en");
    if(t2 == NULL || t2->type != cJSON_Number){
        return  (*env)->NewStringUTF(env,err);
    }
    else{
        req.high_enable = t2->valueint;
    }

    t2 = cJSON_GetObjectItem(t1,"ch_r");
    EV_LOGD("CH_r:t2->type=%d\n",t2->type);

    if(t2 != NULL && t2->type == cJSON_Array){
        for(i = 0;i < 16;i++){
            t3 = cJSON_GetArrayItem(t2,i);
            if(t3 != NULL && t3->type == cJSON_Object){
                t4 = cJSON_GetObjectItem(t3,"value");
                if(t4 != NULL && t4->type == cJSON_Number){
                        req.ch_r[i] = t4->valueint;
                }
            }
        }
    }

    t2 = cJSON_GetObjectItem(t1,"ch_d");
    if(t2 != NULL && t2->type == cJSON_Array){
        for(i = 0;i < 16;i++){
            t3 = cJSON_GetArrayItem(t2,i);
            if(t3 != NULL && t3->type == cJSON_Object){
                t4 = cJSON_GetObjectItem(t3,"value");
                if(t4 != NULL && t4->type == cJSON_Number){
                        req.ch_d[i] = t4->valueint;
                }
            }
        }
    }

    cJSON_Delete(root); //释放root

    res = EV_mdbCoinConfig(&req,&rpt);

    root=cJSON_CreateObject();
    entry = cJSON_CreateObject();
    cJSON_AddItemToObject(root, JSON_HEAD, entry);
    cJSON_AddNumberToObject(entry,JSON_TYPE,EV_MDB_C_CON);

    cJSON_AddNumberToObject(entry,"port_id",rpt.fd);
    cJSON_AddNumberToObject(entry,"acceptor",rpt.acceptor);
    cJSON_AddNumberToObject(entry,"dispenser",rpt.dispenser);
    cJSON_AddNumberToObject(entry,"hight_en",rpt.high_enable);
    if(res == 1){
        cJSON_AddNumberToObject(entry,"is_success",rpt.com_ok);
        cJSON_AddNumberToObject(entry,"result",rpt.res);
    }
    else{
        cJSON_AddNumberToObject(entry,"is_success",0);
        cJSON_AddNumberToObject(entry,"result",0);
    }

    text = cJSON_Print(root);
    cJSON_Delete(root);
    if(text != NULL){
        msg = (*env)->NewStringUTF(env,text);
        free(text);

    }
    else{
        msg = (*env)->NewStringUTF(env,"{\"EV_json\":{\"EV_type\":9999}}");
    }
    return msg;
}

