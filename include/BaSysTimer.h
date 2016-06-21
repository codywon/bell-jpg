
#ifndef _BA_SYS_TIMER_H_
#define _BA_SYS_TIMER_H_



#define MAX_BA_SYS_TIMER_SECOND         1
#define MAX_BA_SYS_TIMER_USECOND        0

#define MAX_BA_SYS_TIMER_COUNT          128

typedef void(*BaSysTimerCallback)(unsigned int sit);

typedef struct
{
    unsigned int nSit;
    unsigned int nTimer;
    unsigned int nInterval;
    BaSysTimerCallback callback;
}stBaSysTimer;


extern void BaSysTimerInit( void );
extern void RegisterBaSysCallback(unsigned int sit, unsigned int interval, BaSysTimerCallback callback);
extern void UnRegisterBaSysCallback(unsigned int sit, BaSysTimerCallback callback);

#define BaUSleep(usec)    \
{ \
    struct timeval tempval;     \
    tempval.tv_sec = 0;         \
    tempval.tv_usec = usec;     \
    select(0, NULL, NULL, NULL, &tempval);      \
}


#endif


