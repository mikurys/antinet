// LICENCE: based on  http://www.boost.org/doc/libs/1_37_0/doc/html/interprocess/synchronization_mechanisms.html#interprocess.synchronization_mechanisms.semaphores

// #include <boost/interprocess/ipc/message_queue.hpp>
#include <iostream>
#include <fstream>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


#include "../counter.hpp"

#include "config.h"


using namespace std;


int main ()
{

	c_counter counter    (std::chrono::seconds(1),true);
	c_counter counter_big(std::chrono::seconds(10),true);
	c_counter counter_all(std::chrono::seconds(999999),true);

	int thefile = open("foo.pipe", O_RDONLY );
	_info("Opened the file as thefile=" << thefile);
	if (thefile<0) { _warn("Can not open pipe"); return 1 ; }
	// ifstream thefile("foo.pipe");

	while (1) {
		std::cout << "Starting reads" << std::endl;

	// t_onemsg __attribute__ ((aligned (256)))   onemsg;
	t_onemsg  onemsg;

   try{

			//while(1) {
      for(int i = 0; i < 1*1000*1000; ++i){

				// thefile.read( reinterpret_cast<char*>( &onemsg ), sizeof(onemsg));
				auto ret = read( thefile, reinterpret_cast<char*>( &onemsg ), sizeof(onemsg) );
				size_t size_packets = ret;

         if (size_packets != sizeof(onemsg)) {
         	 throw std::runtime_error("Receive problem, invalid size");
       	 }
         if (onemsg[7] != 42) {
         	 throw std::runtime_error("Receive problem, invalid MARKER");
       	 }

				bool printed=false;
				printed = printed || counter.tick(size_packets, std::cout);
				bool printed_big = counter_big.tick(size_packets, std::cout);
				printed = printed || printed_big;
				//if (printed_big) packet_check.print();
				counter_all.tick(size_packets, std::cout, true);

      }
			//}
   }
   catch(std::exception &ex){
      std::cout << ex.what() << std::endl;
      return 1;
   }

		if (counter_all.get_bytes_all() > (30*1024*1024*1024LL)) break;

	}

   return 0;
}

