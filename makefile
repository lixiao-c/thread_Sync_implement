all:
	g++ -o synchash_test synchash_test.cpp -lpthread -std=c++11 -D LAMPORT
	g++ -o lockhash_test lockhash_test.cpp -lpthread -std=c++11 -D OPT_MUTEX
	g++ -o counter counter.cpp -lpthread -std=c++11 -D OPT_MUTEX
