#include<linux/futex.h>
#include <sys/syscall.h>
#include<sys/time.h>
#include <unistd.h>  
#include <sys/ipc.h>  
#include <sys/mman.h>  
#include <sys/types.h>   
#include <sys/wait.h>  
#include <sys/stat.h>  
#include <fcntl.h>  

unsigned short xchg_8(void *ptr, unsigned char x)
{
    __asm__ __volatile__("xchgb %0,%1"
                :"=r" (x)
                :"m" (*(volatile unsigned char *)ptr), "0" (x)
                :"memory");
    return x;
}
unsigned short xchg(void *ptr, unsigned int x)
{
    __asm__ __volatile__("xchg %0,%1"
                :"=r" (x)
                :"m" (*(volatile unsigned int *)ptr), "0" (x)
                :"memory");
    return x;
}

//simple spin lock
#define cpu_relax() asm volatile("pause\n": : :"memory") 
void simple_spin_init(char* lock){
    *lock = 0;
}
void simple_spin_lock(char* lock){
    while (true) {
        if (!xchg_8(lock, 1)) return;
        while (*lock) cpu_relax();
    }
}
void simple_spin_unlock(char* lock){
    asm("mfence");
    xchg_8(lock, 0);
}

// optimized spin lock
typedef struct{
    int threadnum;
    int* waiting;
    char flag;
}spinlock_t;

void spin_init(spinlock_t* lock, int threadnum){
    lock->threadnum = threadnum;
    lock->waiting = (int*)malloc(sizeof(int)*threadnum);
    for(int i = 0; i < threadnum; i++)
        lock->waiting[i] = 0;
    lock->flag = 0;
}
void spin_lock(spinlock_t* lock, int threadid){
    lock->waiting[threadid] = 1;
    char acquire_lock = 1;
    while (lock->waiting[threadid] && acquire_lock) {
        acquire_lock = xchg_8(&lock->flag, 1);
        cpu_relax();
    }
    lock->waiting[threadid] = 0;
}
void spin_unlock(spinlock_t* lock, int threadid){
    asm("mfence");
    int other = (threadid + 1) % lock->threadnum;
    while ((other != threadid) && !lock->waiting[other])
	    other = (other + 1) % lock->threadnum;
    if (other == threadid)
	    xchg_8(&lock->flag, 0); 
    else
	    lock->waiting[other] = 0;
}

//mutex
void mutex_lock(int* lock){
    if(!xchg(lock, 1))
        return;
    else{
        syscall(SYS_futex, lock, FUTEX_WAIT, 1, 0, 0, 0); // lock = 1, goto waiting
        mutex_lock(lock);
    }
}
void mutex_unlock(int* lock){
    asm("mfence");
    xchg(lock, 0);
    syscall(SYS_futex, lock, FUTEX_WAKE, 1, 0, 0, 0); // 唤醒一个线程
}
void mutex_init(int* lock){
    *lock = 0;
}

// opt mutex
typedef struct{
    int threadnum;
    int* waiting;
    int flag;
}mutexlock_t;
void optmutex_lock(mutexlock_t* lock, int threadid){
    /*lock->waiting[threadid] = 1;
    if (!lock->waiting[threadid] || !xchg(&lock->flag, 1)){
        lock->waiting[threadid] = 0;
        return;
    }
    else{
        syscall(SYS_futex, lock, FUTEX_WAIT, 1, 0, 0, 0); // lock = 1, goto waiting
        optmutex_lock(lock, threadid);
    }*/
    lock->waiting[threadid] = 1;
    int acquire_lock = 1;
    while (lock->waiting[threadid] && acquire_lock) {
        acquire_lock = xchg(&lock->flag, 1);
        syscall(SYS_futex, lock, FUTEX_WAIT, 1, 0, 0, 0); // lock = 1, goto waiting
    }
    lock->waiting[threadid] = 0;
}
void optmutex_unlock(mutexlock_t* lock, int threadid){
    asm("mfence");
    int other = (threadid + 1) % lock->threadnum;
    while ((other != threadid) && !lock->waiting[other])
	    other = (other + 1) % lock->threadnum;
    if (other == threadid)
	    xchg(&lock->flag, 0); 
    else
	    lock->waiting[other] = 0;
    syscall(SYS_futex, lock, FUTEX_WAKE, 1, 0, 0, 0); // 唤醒一个线程
}
void optmutex_init(mutexlock_t* lock, int threadnum){
    lock->threadnum = threadnum;
    lock->waiting = (int*)malloc(sizeof(int)*threadnum);
    for(int i = 0; i < threadnum; i++)
        lock->waiting[i] = 0;
    lock->flag = 0;
}

// rwlock
