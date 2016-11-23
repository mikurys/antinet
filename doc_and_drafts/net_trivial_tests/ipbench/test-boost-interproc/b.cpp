// LICENCE: based on  http://www.boost.org/doc/libs/1_37_0/doc/html/interprocess/synchronization_mechanisms.html#interprocess.synchronization_mechanisms.semaphores

#include <boost/interprocess/ipc/message_queue.hpp>
#include <iostream>
#include <vector>

#include "../counter.hpp"

#include "config.h"

using namespace boost::interprocess;

using namespace std;


int main ()
{

	c_counter counter    (std::chrono::seconds(1),true);
	c_counter counter_big(std::chrono::seconds(3),true);
	c_counter counter_all(std::chrono::seconds(999999),true);

	while (1) {
		std::cout << "Starting reads" << std::endl;

	t_onemsg onemsg;

   try{
      //Open a message queue.
      message_queue mq
         (open_only        //only create
         ,"message_queue"  //name
         );

      unsigned int priority;
      std::size_t recvd_size;

			while(1) {
      for(int i = 0; i < 1*1000*1000; ++i){
         mq.receive(&onemsg, sizeof(onemsg), recvd_size, priority);
         if (recvd_size != sizeof(onemsg)) {
         	 cout << "Receive problem, invalid size" << endl;
       	 }

				size_t size_packets = sizeof(onemsg);

				bool printed=false;
				printed = printed || counter.tick(size_packets, std::cout);
				bool printed_big = counter_big.tick(size_packets, std::cout);
				printed = printed || printed_big;
				//if (printed_big) packet_check.print();
				counter_all.tick(size_packets, std::cout, true);

      }
			}
   }
   catch(interprocess_exception &ex){
      message_queue::remove("message_queue");
      std::cout << ex.what() << std::endl;
      return 1;
   }
   message_queue::remove("message_queue");

	}

   return 0;
}

