#include"sync.h"
#include"lock.h"
#include"pthread.h"
#include<iostream>
using namespace std;

#ifdef PATTERSON
#define THREAD_NUM 2
patterson_sync* sync_item;
#elif LAMPORT
#define THREAD_NUM 2
lamport_sync* sync_item;
#elif SIMPLE_SPIN
#define THREAD_NUM 4
char* spinlock;
#elif OPT_SPIN
#define THREAD_NUM 4
spinlock_t* spinlock;
#elif MUTEX
#define THREAD_NUM 4
int* mutexlock;
#elif OPT_MUTEX
#define THREAD_NUM 4
mutexlock_t* mutexlock;
#endif

#define COUNTER_NUM 500000
long counternum;

typedef struct{
	int threadid;
} threadParam_t;

#ifdef PATTERSON
void* counter_patterson(void* param){
    threadParam_t* myparam = (threadParam_t*)param;
    int id = myparam->threadid;
    for(int i = 0; i < COUNTER_NUM; i++){
        patterson_lock(sync_item, id);
        counternum++;
        patterson_unlock(sync_item, id);
    }
}
#elif LAMPORT
void* counter_lamport(void* param){
    threadParam_t* myparam = (threadParam_t*)param;
    int id = myparam->threadid;
    for(int i = 0; i < COUNTER_NUM; i++){
        lamport_lock(sync_item, id);
        counternum++;
        lamport_unlock(sync_item, id);
    }
}
#elif SIMPLE_SPIN
void* counter_spin(void* param){
    threadParam_t* myparam = (threadParam_t*)param;
    int id = myparam->threadid;
    for(int i = 0; i < COUNTER_NUM; i++){
        simple_spin_lock(spinlock);
        counternum++;
        simple_spin_unlock(spinlock);
    }
}
#elif OPT_SPIN
void* counter_optspin(void* param){
    threadParam_t* myparam = (threadParam_t*)param;
    int id = myparam->threadid;
    for(int i = 0; i < COUNTER_NUM; i++){
        spin_lock(spinlock, id);
        counternum++;
        spin_unlock(spinlock, id);
    }
}
#elif MUTEX
void* counter_mutex(void* param){
    threadParam_t* myparam = (threadParam_t*)param;
    int id = myparam->threadid;
    for(int i = 0; i < COUNTER_NUM; i++){
        mutex_lock(mutexlock);
        counternum++;
        mutex_unlock(mutexlock);
    }
}
#elif OPT_MUTEX
void* counter_optmutex(void* param){
    threadParam_t* myparam = (threadParam_t*)param;
    int id = myparam->threadid;
    for(int i = 0; i < COUNTER_NUM; i++){
        optmutex_lock(mutexlock, id);
        counternum++;
        optmutex_unlock(mutexlock, id);
    }
}
#endif

int main(){
    counternum=0;

    #ifdef PATTERSON
    sync_item = (patterson_sync*)malloc(sizeof(patterson_sync));
    patterson_init(sync_item);
    #elif LAMPORT
    sync_item = (lamport_sync*)malloc(sizeof(lamport_sync));
    lamport_init(sync_item, THREAD_NUM);
    #elif SIMPLE_SPIN
    spinlock = (char*)malloc(sizeof(char));
    simple_spin_init(spinlock);
    #elif OPT_SPIN
    spinlock = (spinlock_t*)malloc(sizeof(spinlock_t));
    spin_init(spinlock, THREAD_NUM);
    #elif MUTEX
    mutexlock = (int*)malloc(sizeof(int));
    mutex_init(mutexlock);
    #elif OPT_MUTEX
    mutexlock = (mutexlock_t*)malloc(sizeof(mutexlock_t));
    optmutex_init(mutexlock, THREAD_NUM);
    #endif

    pthread_t addthread[THREAD_NUM];
	threadParam_t Tparam[THREAD_NUM]; 
	for (int i = 0; i < THREAD_NUM; i++){
		Tparam[i].threadid = i;
        #ifdef PATTERSON
		pthread_create(&addthread[i], NULL, counter_patterson, (void*)&Tparam[i]);
        #elif LAMPORT
        pthread_create(&addthread[i], NULL, counter_lamport, (void*)&Tparam[i]);
        #elif SIMPLE_SPIN
        pthread_create(&addthread[i], NULL, counter_spin, (void*)&Tparam[i]);
        #elif OPT_SPIN
        pthread_create(&addthread[i], NULL, counter_optspin, (void*)&Tparam[i]);
        #elif MUTEX
        pthread_create(&addthread[i], NULL, counter_mutex, (void*)&Tparam[i]);
        #elif OPT_MUTEX
        pthread_create(&addthread[i], NULL, counter_optmutex, (void*)&Tparam[i]);
        #endif
	}

    for (int i = 0; i < THREAD_NUM; i++)
		pthread_join(addthread[i],NULL);
    
    cout<<"counter is "<<counternum<<endl;
}