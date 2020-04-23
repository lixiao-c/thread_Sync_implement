#include<iostream>
#include<stdlib.h>
#include<stdio.h>
#include"lock.h"

#define TABLE_SIZE 50
typedef struct __node_t_
{
	int key;
	int value;
	struct __node_t_* next;
}node_t;

typedef struct __bucket_t_
{
	node_t* head;
	#ifdef SIMPLE_SPIN
	char* lock;
	#elif OPT_SPIN
	spinlock_t* lock;
	#elif MUTEX
	int* lock;
	#elif OPT_MUTEX
	mutexlock_t* lock;
	#endif
}bucket_t;

typedef struct __hashtable_t
{
	bucket_t* bucket;
}table_t;

#ifdef SIMPLE_SPIN
void bucket_init_simple_spin(bucket_t* bucket)
{
	bucket->head = NULL;
	bucket->lock = (char*)malloc(sizeof(char));
	simple_spin_init(bucket->lock);
}
#elif OPT_SPIN
void bucket_init_optspin(bucket_t* bucket, int thread_num)
{
	bucket->head = NULL;
	bucket->lock = (spinlock_t*)malloc(sizeof(spinlock_t));
	spin_init(bucket->lock, thread_num);
}
#endif

void bucket_delete(bucket_t* bucket, unsigned int key, int threadid)
{
	#ifdef SIMPLE_SPIN
	simple_spin_lock(bucket->lock);
	#elif OPT_SPIN
	spin_lock(bucket->lock, threadid);
	#endif
	node_t* t = bucket->head;
	if (t != NULL)
	{
		if (t->key == key)
		{
			bucket->head = bucket->head->next;
			free(t);
			#ifdef SIMPLE_SPIN
			simple_spin_unlock(bucket->lock);
			#elif OPT_SPIN
			spin_unlock(bucket->lock, threadid);
			#endif
			return;
		}
	}
	while (t->next != NULL)
	{
		if (t->next->key == key)
		{
			node_t* d = t->next;
			t->next = t->next->next;
			free(d);
			#ifdef SIMPLE_SPIN
			simple_spin_unlock(bucket->lock);
			#elif OPT_SPIN
	        spin_unlock(bucket->lock, threadid);
			#endif
			return;
		}
		else t = t->next;
	}
	#ifdef SIMPLE_SPIN
	simple_spin_unlock(bucket->lock);
	#elif OPT_SPIN
	spin_unlock(bucket->lock, threadid);
	#endif
}

node_t* bucket_lookup(bucket_t* bucket, unsigned int key)
{
	node_t* curr = bucket->head;
	while (curr != NULL)
	{
		if (curr->key == key)
		{
			break;
		}
		curr = curr->next;
	}
	return curr;
}

int bucket_search(bucket_t* bucket, unsigned int key, int threadid)
{
	int re_value;
	#ifdef SIMPLE_SPIN
	simple_spin_lock(bucket->lock);
	#elif OPT_SPIN
	spin_lock(bucket->lock, threadid);
	#endif
	node_t* curr = bucket->head;
	while (curr != NULL)
	{
		if (curr->key == key)
		{
			break;
		}
		curr = curr->next;
	}
	if (curr == NULL)
		re_value = -1;
	re_value = curr->value;
	#ifdef SIMPLE_SPIN
	simple_spin_unlock(bucket->lock);
	#elif OPT_SPIN
	spin_unlock(bucket->lock, threadid);
	#endif
	return re_value;
}

void bucket_insert(bucket_t* bucket, unsigned int key, unsigned int value, int threadid)
{
	#ifdef SIMPLE_SPIN
	simple_spin_lock(bucket->lock);
	#elif OPT_SPIN
	spin_lock(bucket->lock, threadid);
	#endif
	node_t* old_node = bucket_lookup(bucket, key);
	if (old_node) {
		old_node->value = value;
		#ifdef SIMPLE_SPIN
		simple_spin_unlock(bucket->lock);
		#elif OPT_SPIN
	    spin_unlock(bucket->lock, threadid);
	    #endif
		return;
	}

	node_t* new_node = (node_t*)malloc(sizeof(node_t));
	if (new_node == NULL)
	{
		perror("malloc");
		#ifdef SIMPLE_SPIN
		simple_spin_unlock(bucket->lock);
		#elif OPT_SPIN
		spin_unlock(bucket->lock, threadid);
		#endif
		return;
	}
	new_node->key = key;
	new_node->value = value;

	new_node->next = bucket->head;
	bucket->head = new_node;
	#ifdef SIMPLE_SPIN
	simple_spin_unlock(bucket->lock);
	#elif OPT_SPIN
	spin_unlock(bucket->lock, threadid);
	#endif
}

int hashfunc(int key) {
	return key%TABLE_SIZE;
}

void table_init(table_t* table, int thread_num) {
	table->bucket = (bucket_t*)malloc(TABLE_SIZE * sizeof(bucket_t));
	for (int i = 0; i < TABLE_SIZE; i++){
		#ifdef SIMPLE_SPIN
		bucket_init_simple_spin(&table->bucket[i]);
		#elif OPT_SPIN
		bucket_init_optspin(&table->bucket[i], thread_num);
		#endif
	}
}

void table_insert(table_t* table, int key, int value, int threadid) {
	int bucket_num = hashfunc(key);
	bucket_insert(&table->bucket[bucket_num], key, value, threadid);
}

int table_search(table_t* table, int key, int threadid) {
	int bucket_num = hashfunc(key);
	return bucket_search(&table->bucket[bucket_num], key, threadid);
}
