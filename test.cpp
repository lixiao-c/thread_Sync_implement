#include"hash_sync.h"
#include"pthread.h"
#include <cstdlib>
#include <random>
#include <ctime>
#include <sys/time.h> 

typedef struct{
	int threadid;
} threadParam_t;

const int threadnum = 2;
const int testnum = 100;
table_t* newtable;

void* patterson_func(void* param){
	threadParam_t* myparam = (threadParam_t*)param;
	srand(time(0)+myparam->threadid);
	for(int i = 0; i < 1000; i++){
		table_insert(newtable, rand()%1000, myparam->threadid, myparam->threadid);
	}
}

int main() {
	struct timeval v1, v2;

	newtable = (table_t*)malloc(sizeof(table_t));
	table_init(newtable, threadnum);
	
	pthread_t insert_thread[threadnum];
	threadParam_t Tparam[threadnum]; 
	gettimeofday(&v1, NULL);
	for (int i = 0; i < threadnum; i++){
		Tparam[i].threadid = i;
		pthread_create(&insert_thread[i], NULL, patterson_func, (void*)&Tparam[i]);
	}

	for (int i = 0; i < threadnum; i++)
		pthread_join(insert_thread[i],NULL);
	gettimeofday(&v2, NULL);

    printf("diff:  sec --- %ld, usec --- %ld\n", (v2.tv_sec - v1.tv_sec), (v2.tv_usec - v1.tv_usec));

	/*for (int i = 40; i < 60; i++) {
		cout << " " << table_search(newtable, i, 0);
	}*/

	return 0;
}
