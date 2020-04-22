#include<iostream>

// patterson 
typedef struct {
    bool want[2];
    int turn;
}patterson_sync;
void patterson_init(patterson_sync* sync_item){
    sync_item->turn = 0;
    sync_item->want[0] = false;
    sync_item->want[1] = false;
}
void patterson_lock(patterson_sync* sync_item, int id){
    sync_item->want[id] = true;
    int j = (id + 1) % 2;
    sync_item->turn = j; // 谦让
    //sync_item->turn = id; // 不谦让
    asm("mfence"); // load store顺序执行
    while (sync_item->want[j] && sync_item->turn == j);
    return;
}
void patterson_unlock(patterson_sync* sync_item, int id){
    asm("mfence");
    sync_item->want[id] = false;
}

//lamport baker
typedef struct {
    int* tickets;
    int* choosing;
    int thread_num;
}lamport_sync;
void lamport_init(lamport_sync* sync_item, int thread_num){
    sync_item->thread_num = thread_num;
    sync_item->tickets = (int*)malloc(sizeof(int)*thread_num);
    sync_item->choosing = (int*)malloc(sizeof(int)*thread_num);
    for(int i = 0; i < thread_num; i++){
        sync_item->tickets[i] = 0;
        sync_item->choosing[i] = 0;
    }
}
void lamport_lock(lamport_sync* sync_item, int threadid){
    sync_item->choosing[threadid] = 1;
    asm("mfence");
    int max_ticket = 0;
    for (int i = 0; i < sync_item->thread_num; i++) {
        int ticket = sync_item->tickets[i];
        max_ticket = ticket > max_ticket ? ticket : max_ticket;
    }
    sync_item->tickets[threadid] = max_ticket + 1;
    asm("mfence"); // barrier
    sync_item->choosing[threadid] = 0;
    asm("mfence");
    for (int other = 0; other < sync_item->thread_num; other++) {
        while (sync_item->choosing[other]);
        asm("mfence");
        while (sync_item->tickets[other] != 0 &&
               (sync_item->tickets[other] < sync_item->tickets[threadid] ||
                (sync_item->tickets[other] == sync_item->tickets[threadid] && other < threadid)));
    }
}
void lamport_unlock(lamport_sync* sync_item, int threadid){
    asm("mfence");
    sync_item->tickets[threadid] = 0;
}