// LICENCE: based on  http://www.boost.org/doc/libs/1_37_0/doc/html/interprocess/synchronization_mechanisms.html#interprocess.synchronization_mechanisms.semaphores

// #include <boost/interprocess/ipc/message_queue.hpp>
#include <iostream>
#include <vector>
#include <fstream>
#include <exception>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <cstring>
#include <cstdlib>

#include <thread>
#include <atomic>
#include <mutex>

#include "../counter.hpp"
#include "config.h"

using namespace std;
using namespace boost::interprocess;

#define _fail(X) { _info(X); throw std::runtime_error("Error"); }


void* dbg_memset(bool dbg,  unsigned char* data, unsigned char pat, size_t size) {
	if (dbg) _info("Will memset with pat="<<(int)pat<<" at data="<<(void*)data<<" size="<<size);
	auto ret = std::memset( const_cast<unsigned char*>(data), pat, size);
	return ret;
}

/*


0    4
|             9004-v
|....................................
|.hdr. data        .hdr.    data    .
|....................................
+------------------------------------
^ shm_ptr
^ msg_ptr (msg0)
^ msg_header (msg0)
     ^ msg_data (msg0)
     ^ packet_data


*/

/*

Runs the program:
@param program_is_producer is true then produces data, is false then consumes

*/

typedef enum {
	e_ipc_shm_unsafe = 1, // using SHM without any proper synchronization, works only if SHM writes/reads are 100% consistent. Probably UB.
} t_ipc_type;

typedef enum {
	e_data_send_zerofill = 1, // zero-fill entire buffer
} t_data_send_type;

typedef enum {
	e_data_recv_check_few = 1, // check only few marker(s), e.g. 1-3 bytes per entire buffer
} t_data_send_type;

template <t_ipc_type T_IPC_TYPE, t_data_send_type T_SEND_TYPE>
int the_program(bool program_is_producer)
{
	_info("Starting program as: " << ( program_is_producer ? "(producent, creates SHM, writes data)" : "(consumer, opens SHM, waits and reads data)" ) );

	string shm_name = "MySharedMemory";

	const int config_thread_count = 2; // number of threads / SHM regions
	const int config_header_size = 4; // bytes for header e.g. the flag

	_info("Create shm... (name: '" << shm_name<<"')");
	shared_memory_object shm;
	if (program_is_producer) {
		shm = shared_memory_object( create_only, shm_name.c_str(), read_write);
	}
	else {
		shm = shared_memory_object( open_only, shm_name.c_str(), read_write);
	}

	// for packet 9000, for 2 thread (header is 4): (9000+4)*2 = 18008
	size_t shm_size_request = (config_packet_size + config_header_size) * config_thread_count;
	_info("Truncate shm to size=" << shm_size_request);
	shm.truncate( shm_size_request );
	_info("Map region shm...");
	mapped_region region(shm, read_write);
	volatile unsigned char* shm_ptr = static_cast<unsigned char*>( region.get_address() );
	volatile unsigned char* shm_data = shm_ptr; // skip shm header (and align?)
	auto shm_size = region.get_size();
	if (shm_size != shm_size_request) throw runtime_error("Bad shm data size");
	auto shm_data_size = shm_size;

	std::cout << "Starting writes" << std::endl; // <=== CLIENT or SERVER ?
	cout << "Packet size: " << config_packet_size << " B" << endl;

	mutex lock_stats;

	_info("shm: data="<<(void*)shm_data<<" size="<<shm_size);


	auto main_loop = [&](int thread_nr) -> int {
		// atomic<unsigned long int> pattern_nr{0};
		unsigned long int pattern_nr{0}; // <=== pattern is per-thread now, not global!

	// --- for thread ---

	c_counter counter    (std::chrono::seconds(1),true);
	c_counter counter_big(std::chrono::seconds(10),true);
	c_counter counter_all(std::chrono::seconds(999999),true);

	const size_t msg_size = (config_packet_size + config_header_size); // e.g. 9000+4 = 9004
	volatile unsigned char* msg_ptr      = shm_data + msg_size*thread_nr; // TODO +offset(pre thread), ptr to: "[flags][packet-data]"
	volatile unsigned char* msg_header   = msg_ptr + 0; // ptr to: "[flags]", e.g. shm+0
	volatile unsigned char* packet_data  = msg_ptr + config_header_size; // TODO+offset(per thread), e.g. shm+4
	const size_t packet_size  = config_packet_size;

	{
		lock_guard<mutex> lg(lock_stats);
		_info("msg:  ptr="<<(void*)msg_ptr<<" size="<<msg_size);
		_info("pck: data="<<(void*)packet_data<<" size="<<packet_size);
	}

	if (program_is_producer) {
		{
			lock_guard<mutex> lg(lock_stats);
			_info("I am the client - writting flag");
		}
		*(msg_header+0) = shflag_owner_writer; // mark - we use this SHM-msg
	}

	const bool dbg_pat=0; // pattern
	const bool dbg_si=0; // spinlock index

	unsigned long int loop_nr=0; // number of loop/packet in this thread, always must be from 0

	try {

		if (program_is_producer) {
			while(1) {
				++pattern_nr;
				++loop_nr;
				unsigned char pat = pattern_nr%256;
				auto mark1 = (pattern_nr/256) % 256; // mark
				auto mark2 = (pat+42) % 256; // mark
				//_info("pattern_nr="<<pattern_nr<<": mark1="<<mark1<<" mark2="<<mark2);

				//_info("before wait spinlock");
				long long int si=0;
				while (*(msg_header+0) != shflag_owner_writer) { ++si; }; // spinlock untill this memory belongs to me
				if (dbg_si) _info("si="<<si);
				//_info("after wait spinlock");

				// write it:
				dbg_memset(dbg_pat , const_cast<unsigned char*>(packet_data), pat, packet_size);
				//unsigned int s = shm_data_size;
				//for (unsigned int i=0; i<s; ++i) shm_data[i] = pat;

				*(packet_data+7) = mark1; // mark
				*(packet_data+99) = mark2; // mark

				//_info("before set spinlock");
				*(msg_header+0) = shflag_owner_reader; // mark - for TUN
				//_info("after set spinlock");

				{
					// lock_guard<mutex> lg(lock_stats); -- counter is threadlocal now XXX
					// but still this is an small race, because .tick() can use cout XXX TODO add silent_tick()

					// ship first iteration time (maybe we waited for other program to start):
					if (loop_nr <= 2) { counter.reset_time(); counter_big.reset_time(); counter_all.reset_time(); }

					auto size_packets = packet_size;
					bool printed=false;
					printed = printed || counter.tick(size_packets, std::cout);
					bool printed_big = counter_big.tick(size_packets, std::cout);
					printed = printed || printed_big;
					//if (printed_big) packet_check.print();
					counter_all.tick(size_packets, std::cout, true);
				}

				auto all = counter_all.get_bytes_all();
				if (all > (config_test_maxsize + 100)) {
					lock_guard<mutex> lg(lock_stats);
					_info("mt#"<<thread_nr<<" will exit, since we processed all="<<all<<" B.");
					break;
				}
			} // all packets
		} // program is client
		else { // program is server
			long int checksum_foo1=0;
			while (1) {
				++pattern_nr;
				++loop_nr;
				unsigned char pat = pattern_nr%256;
				auto mark1 = (pattern_nr/256) % 256; // mark
				auto mark2 = (pat+42) % 256; // mark


				size_t size_packets=0;

				long long int si=0;
				while (*(msg_header+0) != shflag_owner_reader) { ++si; }; // spinlock untill this memory belongs to me
				if (dbg_si) _info("si="<<si);

				// use our mem:
				if ( *(packet_data+7) != mark1) {
					_fail("Receive problem, invalid MARKER mark1="<<mark1<<" in pattern_nr="<<pattern_nr);
				}
				if ( *(packet_data+99) != mark2) {
					_fail("Receive problem, invalid MARKER mark2="<<mark2<<" in pattern_nr="<<pattern_nr);
				}

				{ // read random position of pattern
					long int pos;
					do { // find a pos that is not a marker
						pos = rand() % packet_size;
					} while (!( (pos!=7) && (pos!=99)  )
					);

					auto addr = packet_data + pos;

					if (!( *addr == pat )) {
						_info("Invalid pattern, while pattern_nr="<<pattern_nr<<" at pos="<<(int)pos
							<<" at addr="<<addr<<", data was: " << static_cast<int>(*addr)
							<<", expected: " << static_cast<int>(pat) );
						throw std::runtime_error("Receive problem, invalid data");
					}
				}

				{
					unsigned char acc = 0;
					//uint_fast32_t acc = 0;

					for (uint_fast16_t i=0; i < packet_size; i += sizeof(acc)) {
						acc ^= * reinterpret_cast<volatile decltype(acc) *>( & packet_data[i] );
					}


					//for(unsigned char d : (unsigned int i=0; i < packet_size; ++i) acc = acc ^ packet_data[i];
					uint8_t acc_byte = 0;
					// take e.g. 4 iterations, for 4 times 1-byte value of 4-byte (32-bit) long accumulator:
					for (uint8_t part=0; part<sizeof(acc)/sizeof(acc_byte); ++part) {
						acc_byte ^= acc >> (part*8);
					}
					// _info("acc_byte=" << (int)acc_byte << " from acc="<<acc << " on pattern_nr=" << pattern_nr);
					if (acc_byte == 123) ++checksum_foo1;
				}


				size_packets += packet_size;

				// end of read:
				*(msg_header+0) = shflag_owner_writer; // semafor

				{
					// lock_guard<mutex> lg(lock_stats); -- counter is threadlocal now XXX
					// but still this is an small race, because .tick() can use cout XXX TODO add silent_tick()

					bool printed=false;
					printed = printed || counter.tick(size_packets, std::cout);
					bool printed_big = counter_big.tick(size_packets, std::cout);
					printed = printed || printed_big;
					//if (printed_big) packet_check.print();
					counter_all.tick(size_packets, std::cout, true);
				}

				auto all = counter_all.get_bytes_all();
				if (all > (config_test_maxsize)) {
					lock_guard<mutex> lg(lock_stats);
					_info("mt#"<<thread_nr<<" will exit, since we processed all="<<all<<" B.");
					break;
				}

			} // all packets
			_info("checksum_foo1 = " << checksum_foo1);
		} // program is server
	} // try
	catch(std::exception &ex){
		std::cout << ex.what() << std::endl;
		return 1;
	}

	{
		lock_guard<mutex> lg(lock_stats);
		_info("Exiting thread #" << thread_nr);
	}
	counter_all.print(std::cout);

	return 0; // no error
	} ; // thread main loop


	// --- start the threads ---

	{
		lock_guard<mutex> lg(lock_stats);
		_info("Starting threads...");
	}
	vector<thread> threads;
	threads.push_back( move( thread(main_loop , 0)));
	threads.push_back( move( thread(main_loop , 1)));
	{
		lock_guard<mutex> lg(lock_stats);
		_info("Threads are running.");
	}

	for (auto & th : threads) th.join();
	{
		lock_guard<mutex> lg(lock_stats);
		_info("Threads are all joined now.");
	}

	// --- single threaded ---

	cout << "Packet size: " << config_packet_size << " B" << endl;

	std::cout << "All done" << std::endl << "\n\n";
	cout << "Packet size: " << config_packet_size << " B" << " gives: ";
	// counter_all.print(std::cout);
	return 0;
}

int main(int argc, const char** argv) {
	vector<string> argm;
	for (int i=0; i<argc; ++i) argm.push_back(argv[i]);

	string opt_type;
	try {
		opt_type = argm.at(1);
	} catch(...) {
		cerr << "Run program with argument 'a' for the client, and other with argument 'b' for server." << endl;
		return 1;
	}
	int ret =	the_program<e_ipc_shm_unsafe>( opt_type=="a" ); // client or server
	return ret;
}


