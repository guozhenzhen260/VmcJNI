#ifndef _DB_JSON_H_
#define _DB_JSON_H_

#include "db_driver.h"
#include "cJSON.h"

#define JSON_HEAD		"EV_json"
#define JSON_TYPE		"EV_type"
#define JSON_VBOX       "EV_vbox"
#if 0
void Json_insert_str(cJSON *json,char *label,char *value);
void Json_insert_int(cJSON *json,char *label,long value);
cJSON * Json_insert_head(cJSON *entry);


#endif
void Json_create_title(int type);
int JSON_getInt(cJSON *node,const char *text);
char *JSON_getString(cJSON *node,const char *text);
#endif // DB_JSON_H
