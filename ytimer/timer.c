#include <stdio.h>
#include <stdlib.h>
#include "timer.h"

#ifdef _WIN32
#include <windows.h>
#include <pthread.h>
#else
#include <sys/stat.h>
#include <sys/time.h> //定时器
#include <signal.h>
#include <pthread.h>
#endif

static pthread_t timer_pid;
static char timerStopped = 1;

//定义定时器链表
static TIMER_NODE timer_node;
static unsigned int timerSum = 1;
static unsigned char isThreadStarted = 0;

void *EV_timer_run(void *ptr)
{
    TIMER_NODE *q,*p;
    ST_TIMER *timer;
    if(ptr == NULL)
    {
        pthread_exit(0);
        return NULL;
    }

    while(timerStopped == 0)
    {      
        EV_msleep(99);
        p = (TIMER_NODE *)ptr;
        while(p->next != NULL)
        {
            q = p->next;
            p = q;
            timer = q->timer;
            if(timer == NULL) continue;
            if(timer->tick)
            {
                timer->tick--;
            }
            else
            {
                if(timer->isr && timer->start == 1)
                {                  
                    timer->isr();
                    timer->tick = timer->varTick;
                }
            }
        }

    }
    printf("pthread_exit\n");
    pthread_exit(0);
    return NULL;
}



//注册定时器100ms的定时器  成功返回定时器ID号  失败返回 -1
int EV_timer_register(EV_timerISR timer_isr)
{
    int id;
    TIMER_NODE *p,*q,*t;
    ST_TIMER *st_timer;
    //定时器精度为100毫秒级
    if(isThreadStarted == 0)//还没开启线程 线开启定时器线程
    {
        printf("First create thread..\n");
        timer_node.next = NULL;//首节点
        timer_node.timer = NULL;
        timerStopped = 0;
        id = pthread_create(&timer_pid,NULL,EV_timer_run,(void *)&timer_node);
        printf("After create thread:%d\n",id);
        if(id != 0)
        {
            printf("thread create failed..\n");
            return -1;
        }    
        isThreadStarted = 1;
    }
    //创建一个定时器结构体
    st_timer = (ST_TIMER *)malloc(sizeof(ST_TIMER));
    if(st_timer == NULL)
        return -1;
    st_timer->start = 0;
    st_timer->tick = 0;
    st_timer->id = timerSum++;
    st_timer->isr = (EV_timerISR)timer_isr;

    //创建一个定时器结构节点
    t = (TIMER_NODE *)malloc(sizeof(TIMER_NODE));
    if(t == NULL)
        return -1;
    t->next = NULL;
    t->timer = st_timer;
    //插入链表
    p = &timer_node;
    while(p->next != NULL)
    {
        q = p;
        p = q->next;
        printf("ex_timer:%d\n",p->timer->id);
    }
    p->next = t;
    printf("register OK:%d\n",st_timer->id);
    return st_timer->id;
}

//彻底关掉定时器
void EV_timer_release(int timerId)
{
    TIMER_NODE *p = &timer_node,*q,*s;
    while(p->next != NULL)
    {
        s = p->next;
        if(s->timer->id  == timerId)//找到了定时器
        {
            printf("free timer:%d\n",s->timer->id);
            p->next = s->next;//下下一个定时器
            free(s);//释放内存
            break;
        }
        else
        {
            q = p;
            p = q->next;
        }
    }
    p = &timer_node;
    while(p->next != NULL)
    {
        q = p->next;
        p = q;
        printf("re_timer:%d\n",q->timer->id);
    }
    if(timer_node.next == NULL)//没有定时器了 则杀掉线程
    {
        timerStopped = 1;
        isThreadStarted = 0;
        printf("all timer killed\n");
    }

}


void EV_timer_stop(int timerId)
{
    TIMER_NODE *p = &timer_node,*q,*s;
    while(p->next != NULL)
    {
        s = p->next;
        if(s->timer->id  == timerId)//找到了定时器
        {
            printf("free timer:%d\n",s->timer->id);
            s->timer->start = 0;
            return;
        }
        else
        {
            q = p;
            p = q->next;
        }
    }
}



uint8 EV_timer_isTimeout(int timerId)
{
    TIMER_NODE *p = &timer_node,*q,*s;
    while(p->next != NULL)
    {
        s = p->next;
        if(s->timer->id  == timerId)//找到了定时器
        {
            printf("free timer:%d\n",s->timer->id);
            if(s->timer->tick == 0)
                return 1;
            else
                return 0;
        }
        else
        {
            q = p;
            p = q->next;
        }
    }

    return 1;
}


uint8 EV_timer_start(int timerId,uint32 sec)
{
    TIMER_NODE *p = &timer_node,*q,*s;
    while(p->next != NULL)
    {
        s = p->next;
        if(s->timer->id  == timerId)//找到了定时器
        {
            s->timer->varTick = (sec * 10);
            s->timer->tick = s->timer->varTick;
            s->timer->start = 1;
            return 1;
        }
        else
        {
            q = p;
            p = q->next;
        }
    }
    return 0;
}



void EV_msleep(int msec)//毫秒睡眠
{

#ifdef _WIN32
    Sleep(msec);
#else
    usleep(msec * 1000);
#endif
}







