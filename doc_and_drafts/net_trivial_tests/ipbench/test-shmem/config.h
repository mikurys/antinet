
//#define config_packet_size 100
//#define config_packet_size 1000
#define config_packet_size 9000
//#define config_packet_size 65000

const long long int config_test_maxsize = 100*1024*1024*1024LL*1LL;

// typedef char t_onemsg[100];

char shflag_owner_writer=0; // the program that writes own this buffer
char shflag_owner_reader=1; // the one that reads

