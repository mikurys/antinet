// LICENCE: based on  http://www.boost.org/doc/libs/1_37_0/doc/html/interprocess/synchronization_mechanisms.html#interprocess.synchronization_mechanisms.semaphores

// #include <boost/interprocess/ipc/message_queue.hpp>
#include <iostream>
#include <fstream>
#include <vector>

#include "../counter.hpp"

#include "config.h"

using namespace std;


int main ()
{

	c_counter counter    (std::chrono::seconds(1),true);
	c_counter counter_big(std::chrono::seconds(3),true);
	c_counter counter_all(std::chrono::seconds(999999),true);

	ifstream thefile("foo.pipe");

	while (1) {
		std::cout << "Starting reads" << std::endl;

	t_onemsg onemsg;

   try{

			while(1) {
      for(int i = 0; i < 1*1000*1000; ++i){

				thefile.read( reinterpret_cast<char*>( &onemsg ), sizeof(onemsg));
				size_t size_packets = sizeof(t_onemsg);

         if (size_packets != sizeof(onemsg)) {
         	 cout << "Receive problem, invalid size" << endl;
       	 }

				bool printed=false;
				printed = printed || counter.tick(size_packets, std::cout);
				bool printed_big = counter_big.tick(size_packets, std::cout);
				printed = printed || printed_big;
				//if (printed_big) packet_check.print();
				counter_all.tick(size_packets, std::cout, true);

      }
			}
   }
   catch(std::exception &ex){
      std::cout << ex.what() << std::endl;
      return 1;
   }

	}

   return 0;
}

