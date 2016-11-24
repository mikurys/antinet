// LICENCE: based on  http://www.boost.org/doc/libs/1_37_0/doc/html/interprocess/synchronization_mechanisms.html#interprocess.synchronization_mechanisms.semaphores

// #include <boost/interprocess/ipc/message_queue.hpp>
#include <iostream>
#include <vector>
#include <fstream>

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

	c_counter counter    (std::chrono::seconds(1),true);
	c_counter counter_big(std::chrono::seconds(10),true);
	c_counter counter_all(std::chrono::seconds(999999),true);

	_info("Create shm...");
	shared_memory_object shm (create_only, "MySharedMemory", read_write);
	_info("Truncate shm...");
	shm.truncate(config_packet_size);
	_info("Map region shm...");
	mapped_region region(shm, read_write);
	volatile unsigned char* shm_ptr = static_cast<unsigned char*>( region.get_address() );
	volatile unsigned char* shm_data = shm_ptr + 4; // skip shm header (and align?)
	auto shm_size = region.	get_size();
	auto shm_data_size = shm_size - 4;

	*(shm_ptr+0) = shflag_owner_writer; // mark - for us

	_info("shm: data="<<(void*)shm_data<<" size="<<shm_size);

	std::cout << "Starting writes" << std::endl;
	unsigned long int pattern_nr=0;

	const bool dbg_pat=0; // pattern
	const bool dbg_si=0; // spinlock index

	try {
		while(1) {
			++pattern_nr;

				//_info("before wait spinlock");
				long long int si=0;
				while (*(shm_ptr+0) != shflag_owner_writer) { ++si; }; // spinlock untill this memory belongs to me
				if (dbg_si) _info("si="<<si);
				//_info("after wait spinlock");

				// write it:
				unsigned char pat = pattern_nr%256;
				std::memset( const_cast<unsigned char*>(shm_data), pat, shm_data_size);
				//unsigned int s = shm_data_size;
				//for (unsigned int i=0; i<s; ++i) shm_data[i] = pat;
				if (dbg_pat) _info("At pattern_nr="<<pattern_nr<<": filled with pat=" << static_cast<int>(pat)
					<< " size: "<<shm_data_size<<" from shm_data="<<(void*)shm_data);

				*(shm_data+7) = 42; // mark
				*(shm_data+99) = 99 %256; // mark
				//_info("before set spinlock");
				*(shm_ptr+0) = shflag_owner_reader; // mark - for TUN
				//_info("after set spinlock");

				auto size_packets = shm_size;
				bool printed=false;
				printed = printed || counter.tick(size_packets, std::cout);
				bool printed_big = counter_big.tick(size_packets, std::cout);
				printed = printed || printed_big;
				//if (printed_big) packet_check.print();
				counter_all.tick(size_packets, std::cout, true);

			if (counter_all.get_bytes_all() > (30*1024*1024*1024LL + 100)) break;
		} // all packets
	} // try
	catch(std::exception &ex){
		std::cout << ex.what() << std::endl;
		return 1;
	}


	std::cout << "All done" << std::endl << "\n\n";
	counter_all.print(std::cout);
	return 0;
}
