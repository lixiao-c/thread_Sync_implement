#include<iostream>
#include<stdlib.h>
#include<stdio.h>
using namespace std;

#define TABLE_SIZE 100
typedef struct __node_t_
{
	int key;
	int value;
	struct __node_t_* next;
}node_t;

typedef struct __bucket_t_
{
	node_t* head;
}bucket_t;

typedef struct __hashtable_t
{
	bucket_t* bucket;
}table_t;

void bucket_init(bucket_t* bucket)
{
	bucket->head = NULL;
}

void bucket_delete(bucket_t* bucket, unsigned int key)
{
	node_t* t = bucket->head;
	if (t != NULL)
	{
		if (t->key == key)
		{
			bucket->head = bucket->head->next;
			free(t);
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
			return;
		}
		else t = t->next;
	}
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

int bucket_search(bucket_t* bucket, unsigned int key)
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
	if (curr == NULL)
		return -1;
	return curr->value;
}

void bucket_insert(bucket_t* bucket, unsigned int key, unsigned int value)
{
	node_t* old_node = bucket_lookup(bucket, key);
	if (old_node) {
		old_node->value = value;
		return;
	}

	node_t* new_node = (node_t*)malloc(sizeof(node_t));
	if (new_node == NULL)
	{
		perror("malloc");
		return;
	}
	new_node->key = key;
	new_node->value = value;

	new_node->next = bucket->head;
	bucket->head = new_node;

}

int hashfunc(int key) {
	return key%TABLE_SIZE;
}

void table_init(table_t* table) {
	table->bucket = (bucket_t*)malloc(TABLE_SIZE * sizeof(bucket_t));
	for (int i = 0; i < TABLE_SIZE; i++)
		bucket_init(&table->bucket[i]);
}

void table_insert(table_t* table, int key, int value) {
	int bucket_num = hashfunc(key);
	bucket_insert(&table->bucket[bucket_num], key, value);
}

int table_search(table_t* table, int key) {
	int bucket_num = hashfunc(key);
	return bucket_search(&table->bucket[bucket_num], key);
}
