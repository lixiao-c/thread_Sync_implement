## Thread synchronization mechanism implementation
This project implments some synchronization mechanism, including spinlock, mutex lock, lamport baker and patterson.
- lock.h spinlock, optimizated spinlock, mutex and optimized mutex
- sync.h lamport baker and patterson

There are two examples that leverage these synchronization mechanism. 

- counter

Several threads add a counter

- concurrent chain hash table

every bucket of the hash table is parotected by these synchronization mechanism (for example, spin lock).

