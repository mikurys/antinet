all:
	g++ main.cpp -lboost_system -lboost_thread

run:
	LD_PRELOAD=/home/rysiek/test/io_service_test/libpreload.so ./a.out

malloc:
	gcc -fPIC -shared  -o libpreload.so malloc.c -ldl
