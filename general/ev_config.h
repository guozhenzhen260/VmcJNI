#ifndef _EV_CONFIG_H_
#define _EV_CONFIG_H_
#include "LOGC.h"

//适用于安卓的特定定位输出
#if (defined EV_ANDROID)
#include<android/log.h>
#define EV_LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "EV_thread", __VA_ARGS__))
#define EV_LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "EV_thread", __VA_ARGS__))
#define EV_LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "EV_thread", __VA_ARGS__))
#define EV_LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "EV_thread", __VA_ARGS__))
#define EV_LOGCOM(...) ((void)__android_log_print(ANDROID_LOG_INFO, "EV_thread", __VA_ARGS__))
#define EV_LOGTASK(...) ((void)__android_log_print(ANDROID_LOG_INFO, "EV_thread", __VA_ARGS__))
#define EV_LOGFLOW(...) ((void)__android_log_print(ANDROID_LOG_INFO, "EV_thread", __VA_ARGS__))

#elif (defined EV_WIN32)
#define EV_LOGD(...)    (DebugLog( __FILE__ , __LINE__ , __VA_ARGS__ ))
#define EV_LOGI(...)    (InfoLog( __FILE__ , __LINE__ , __VA_ARGS__ ))
#define EV_LOGW(...)    (WarnLog( __FILE__ , __LINE__ , __VA_ARGS__ ))
#define EV_LOGE(...)    (ErrorLog( __FILE__ , __LINE__ , __VA_ARGS__ ))
#define EV_LOGCOM(...)  (InfoLog( __FILE__ , __LINE__ , __VA_ARGS__ ))
#define EV_LOGTASK(...) (InfoLog( __FILE__ , __LINE__ , __VA_ARGS__ ))
#define EV_LOGFLOW(...) (InfoLog( __FILE__ , __LINE__ , __VA_ARGS__ ))

#else
#define EV_LOGD(...)    do{}while(0)
#define EV_LOGI(...)    do{}while(0)
#define EV_LOGW(...)    do{}while(0)
#define EV_LOGE(...)    do{}while(0)
#define EV_LOGCOM(...)  do{}while(0)
#define EV_LOGTASK(...) do{}while(0)
#define EV_LOGFLOW(...) do{}while(0)

#endif




int EV_createLog();
#endif
