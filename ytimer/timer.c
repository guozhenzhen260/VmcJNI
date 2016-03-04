#include <stdio.h>
#include <stdlib.h>
#include "timer.h"

#ifdef _WIN32
#include <windows.h>
#include <pthread.h>
#else
#include <sys/stat.h>
#include <sys/time.h> //��ʱ��
#include <signal.h>
#include <pthread.h>
#endif

static pthread_t timer_pid;
static char timerStopped = 1;

//���嶨ʱ������
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



//ע�ᶨʱ��100ms�Ķ�ʱ��  �ɹ����ض�ʱ��ID��  ʧ�ܷ��� -1
int EV_timer_register(EV_timerISR timer_isr)
{
    int id;
    TIMER_NODE *p,*q,*t;
    ST_TIMER *st_timer;
    //��ʱ������Ϊ100���뼶
    if(isThreadStarted == 0)//��û�����߳� �߿�����ʱ���߳�
    {
        printf("First create thread..\n");
        timer_node.next = NULL;//�׽ڵ�
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
    //����һ����ʱ���ṹ��
    st_timer = (ST_TIMER *)malloc(sizeof(ST_TIMER));
    if(st_timer == NULL)
        return -1;
    st_timer->start = 0;
    st_timer->tick = 0;
    st_timer->id = timerSum++;
    st_timer->isr = (EV_timerISR)timer_isr;

    //����һ����ʱ���ṹ�ڵ�
    t = (TIMER_NODE *)malloc(sizeof(TIMER_NODE));
    if(t == NULL)
        return -1;
    t->next = NULL;
    t->timer = st_timer;
    //��������
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

//���׹ص���ʱ��
void EV_timer_release(int timerId)
{
    TIMER_NODE *p = &timer_node,*q,*s;
    while(p->next != NULL)
    {
        s = p->next;
        if(s->timer->id  == timerId)//�ҵ��˶�ʱ��
        {
            printf("free timer:%d\n",s->timer->id);
            p->next = s->next;//����һ����ʱ��
            free(s);//�ͷ��ڴ�
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
    if(timer_node.next == NULL)//û�ж�ʱ���� ��ɱ���߳�
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
        if(s->timer->id  == timerId)//�ҵ��˶�ʱ��
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
        if(s->timer->id  == timerId)//�ҵ��˶�ʱ��
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
        if(s->timer->id  == timerId)//�ҵ��˶�ʱ��
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



void EV_msleep(int msec)//����˯��
{

#ifdef _WIN32
    Sleep(msec);
#else
    usleep(msec * 1000);
#endif
}







