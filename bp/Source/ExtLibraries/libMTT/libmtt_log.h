#ifndef __LIBMTT_LOG_H__
#define __LIBMTT_LOG_H__

#include <sys/ppu_thread.h>


#ifdef __cplusplus
extern "C" {
#endif

// Currently can't be called because it is called automaticaly before main.
int mttLogInit(const char* filename);

//This is called automatically when your process exits
// If you call it early, it will flush the log and no more logging will occur.
void mttLogShutdown();

//Adds an ENDFRAME event to the log.
void mttLogNewFrame();

//Puts a custom event marker in the log.
// str: A string describing the custom event.
void mttLogCustomEvent(const char* str);

//Registers information on a thread.
// id: The threadID as obtained from the system.
// stack_size: The thread's stack size.
// name: The thread's name.
void mttSetThreadInfo(sys_ppu_thread_t thread_id, size_t stack_size, const char* name);

#ifdef __cplusplus
}
#endif


#endif
