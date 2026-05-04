#ifndef __BP_THREADING__
#define __BP_THREADING__

#ifdef __cplusplus
extern "C" {
#endif

#define PS2_THREAD_ERROR            (-1)
#define PS2_MAX_THREADS             256
#define PS2_MAX_SEMAPHORES          256
#define PS2_MAX_PRIORITY            128
#define MAX_HANDLERS            128
#define MAX_ALARMS              64

#define THS_RUN                 0x01
#define THS_READY               0x02
#define THS_WAIT                0x04
#define THS_SUSPEND             0x08
#define THS_WAITSUSPEND         0x0c
#define THS_DORMANT             0x10

struct ThreadParam {
    int     status;
    void    (*entry)(void *);
    void    *stack;
    int     stackSize;
    void    *gpReg;
    int     initPriority;
    int     currentPriority;
    u_int   attr;    
    u_int   option;
    int     waitType;
    int     waitId;
    int     wakeupCount;
};

struct SemaParam {
    int     currentCount;
    int     maxCount;
    int     initCount;
    int     numWaitThreads;
    u_int   attr;
    u_int   option;
};

/*
* Multi Thread
*/

int BP_CreateThread(struct ThreadParam *);
int BP_DeleteThread(int);
int BP_StartThread(int, void *arg);
void BP_ExitThread(void);
void BP_ExitDeleteThread(void);
int BP_TerminateThread(int);
int BP_iTerminateThread(int);
int BP_ChangeThreadPriority(int, int);
int BP_iChangeThreadPriority(int, int);
int BP_RotateThreadReadyQueue(int);
int BP_iRotateThreadReadyQueue(int);
int BP_ReleaseWaitThread(int);     
int BP_iReleaseWaitThread(int);
int BP_GetThreadId(void);
int BP_ReferThreadStatus(int, struct ThreadParam *);
int BP_iReferThreadStatus(int, struct ThreadParam *);
int BP_SleepThread(void);
int BP_WakeupThread(int);
int BP_iWakeupThread(int);
int BP_CancelWakeupThread(int);
int BP_iCancelWakeupThread(int);
int BP_SuspendThread(int);
int BP_iSuspendThread(int);
int BP_ResumeThread(int);
int BP_iResumeThread(int);
int BP_DelayThread(u_int);

/*
* Semaphore
*/

int CreateSema(struct SemaParam *, char const *pBP_DebugName);
int DeleteSema(int);
int SignalSema(int);
int iSignalSema(int);
int WaitSema(int);
int PollSema(int);
int iPollSema(int);
int ReferSemaStatus(int, struct SemaParam *);
int iReferSemaStatus(int, struct SemaParam *);

void bp_yield_ps2_thread_to_vsync();

#ifdef __cplusplus
}
#endif

#endif
