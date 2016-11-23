// LICENCE: based on  http://www.boost.org/doc/libs/1_37_0/doc/html/interprocess/synchronization_mechanisms.html#interprocess.synchronization_mechanisms.semaphores

// #include <boost/interprocess/ipc/message_queue.hpp>
#include <iostream>
#include <vector>
#include <fstream>
#include "../counter.hpp"

#include "config.h"

using namespace std;


int main ()
{
	t_onemsg onemsg;

	std::cout << "Starting writes" << std::endl;
	c_counter counter    (std::chrono::seconds(1),true);
	c_counter counter_big(std::chrono::seconds(3),true);
	c_counter counter_all(std::chrono::seconds(999999),true);

	ofstream thefile("foo.pipe");

	while(1) {
		size_t size_packets;

		try{

			while (1) {
			for(long int i = 0; i < 1*1000*1000; ++i){
				thefile.write( reinterpret_cast<char*>( &onemsg ), sizeof(onemsg));
				size_packets = sizeof(t_onemsg);

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

	std::cout << "All done" << std::endl;
	return 0;
}
