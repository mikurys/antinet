// LICENCE: based on  http://www.boost.org/doc/libs/1_37_0/doc/html/interprocess/synchronization_mechanisms.html#interprocess.synchronization_mechanisms.semaphores

// #include <boost/interprocess/ipc/message_queue.hpp>
#include <iostream>
#include <fstream>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <cstring>
#include <cstdlib>


#include "../counter.hpp"
#include "config.h"


using namespace std;
using namespace boost::interprocess;


int main ()
{
	_info("main");

	c_counter counter    (std::chrono::seconds(1),true);
	c_counter counter_big(std::chrono::seconds(10),true);
	c_counter counter_all(std::chrono::seconds(999999),true);

	_info("Create shm...");
	shared_memory_object shm (open_only, "MySharedMemory", read_write);
	_info("Truncate shm...");
	mapped_region region(shm, read_write);
	_info("Map region shm...");
	volatile unsigned char* shm_ptr = static_cast<unsigned char*>( region.get_address() );
	volatile unsigned char* shm_data = shm_ptr + 4; // skip shm header (and align?)
	auto shm_size = region.	get_size();
	auto shm_data_size = shm_size - 4;

	_info("shm: data="<<(void*)shm_data<<" size="<<shm_size);

	std::cout << "Starting reads" << std::endl;
	unsigned long int pattern_nr=0;

	const bool dbg_pat=0; // pattern
	const bool dbg_si=0; // spinlock index

	try {
   while (1) {
   		++pattern_nr;
			size_t size_packets=0;

			long long int si=0;
			while (*(shm_ptr+0) != shflag_owner_reader) { ++si; }; // spinlock untill this memory belongs to me
			if (dbg_si) _info("si="<<si);

			// use our mem:
			 if ( *(shm_data+7) != 42) {
				 throw std::runtime_error("Receive problem, invalid MARKER");
			 }
			if ( *(shm_data+99) != (99%256)) {
				 throw std::runtime_error("Receive problem, invalid MARKER");
			}
			unsigned char pat = pattern_nr%256;
			long int pos;
			do { // find a pos that is not a marker
				pos = rand() % shm_data_size;
			} while (!( (pos!=7) && (pos!=99)  ));
			if (!( *(shm_data + pos) == pat )) {
				_info("Invalid pattern, while pattern_nr="<<pattern_nr<<" at pos="<<(int)pos
					<<" was: "<< static_cast<int>(*(shm_data + pos)) << ", expected: " << static_cast<int>(pat) );
				throw std::runtime_error("Receive problem, invalid data");
			}

			size_packets += shm_size;

			// end of read:
			*(shm_ptr+0) = shflag_owner_writer; // semafor

			bool printed=false;
			printed = printed || counter.tick(size_packets, std::cout);
			bool printed_big = counter_big.tick(size_packets, std::cout);
			printed = printed || printed_big;
			//if (printed_big) packet_check.print();
			counter_all.tick(size_packets, std::cout, true);

		if (counter_all.get_bytes_all() > (30*1024*1024*1024LL)) break;

   } // all packets
  } // try
  catch(std::exception &ex){
		std::cout << ex.what() << std::endl;
		return 1;
	}

   return 0;
}

