#include"hash_lock.h"
#include"pthread.h"
#include <cstdlib>
#include <random>
#include <ctime>
#include <sys/time.h> 
using namespace std;

typedef struct{
	int threadid;
} threadParam_t;

const int threadnum = 8;
const long testnum = 1000000;
const long keyrange = 100000;
table_t* newtable;

void* insert_func(void* param){
	threadParam_t* myparam = (threadParam_t*)param;
	srand(time(0)+myparam->threadid);
	long kvnum = testnum/threadnum;
	for(int i = 0; i < kvnum; i++){
		table_insert(newtable, rand()%keyrange, myparam->threadid, myparam->threadid);
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
		pthread_create(&insert_thread[i], NULL, insert_func, (void*)&Tparam[i]);
	}

	for (int i = 0; i < threadnum; i++)
		pthread_join(insert_thread[i],NULL);
	gettimeofday(&v2, NULL);

	double time =  (v2.tv_sec - v1.tv_sec)*1000+(double)(v2.tv_usec - v1.tv_usec)/1000; // ms
	cout<<"time ms: "<<time<<endl;
	cout<<"throughput: "<<testnum/time<<" kops/s "<<endl;

	/*for (int i = 40; i < 60; i++) {
		cout << " " << table_search(newtable, i, 0);
	}*/

	return 0;
}
