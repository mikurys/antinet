/**
See LICENCE.txt
*/

const char * disclaimer = "*** WARNING: This is a work in progress, do NOT use this code, it has bugs, vulns, and 'typpos' everywhere. ***"; // XXX

#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netdb.h>

#include <string.h>
#include <assert.h>

#include <thread>

#include <cstring>


#include "counter.hpp"

#include "cpputils.hpp" // TODO move to lib later

#include "config.h"

const bool encryption = false;
const unsigned char xorpass = 42;

// TODO move:
void error(const std::string & msg) {
	std::cout << "Error: " << msg << std::endl;
	throw std::runtime_error(msg);
}

// ------------------------------------------------------------------

class c_ipbench {
	public:
		c_ipbench();

		void configure(const std::vector<std::string> & args);
		void run();
		void show_usage() const;

	protected:
		void prepare_socket(); ///< make sure that the lower level members of handling the socket are ready to run
		void event_loop(); ///< the main loop

	private:
		std::string m_target_addr; ///< our target: IP address
		int m_target_port; ///< our target: IP (e.g. UDP) port number
		bool m_target_is_ipv6; ///< is the target's address an IPv6

		int m_blocksize; ///< size of the block to send

		int m_sock; ///< the network socket
		as_zerofill< sockaddr_in  > m_sockaddr4 ; ///< socket address (ipv4)
		as_zerofill< sockaddr_in6 > m_sockaddr6 ; ///< socket address (ipv6)

		int m_multisend; //< send messsages groupd as this many (e.g. sendmmsg). 0==disable, use sendmsg
};

// ------------------------------------------------------------------

using namespace std; // XXX move to implementations, not to header-files later, if splitting cpp/hpp

c_ipbench::c_ipbench() {
	m_blocksize=0;
	memzero(m_sockaddr4);
	memzero(m_sockaddr6);
}

void c_ipbench::show_usage() const {
	auto & out = cout;
	out << "Usage: programname addr port iptype blocksize" << endl
		<< "programname 192.168.1.66 5555 ipv4 1300" << endl
		<< "programname fd00:808:808:808:808:808:808:1111 5555 ipv6 9000" << endl
		<< "programname fd00:808:808:808:808:808:808:808 5555 ipv6 9000" << endl
	;
}

void c_ipbench::configure(const std::vector<std::string> & args) {
	if (args.size()<3) {
		show_usage();
		throw std::runtime_error("Invalid program options");
	}

	m_target_addr = args.at(1);
	m_target_port = atoi(args.at(2).c_str());

	string protocol_name = args.at(3);
	if (protocol_name=="ipv6") m_target_is_ipv6=1;
	else if (protocol_name=="ipv4") m_target_is_ipv6=0;
	else error(string("Bad protocol name '") + protocol_name + "', use instead name: 'ipv6' or 'ipv4'");

	m_blocksize = atoi(args.at(4).c_str());
	assert(m_blocksize >= 1);

	m_multisend = atoi(args.at(5).c_str());
	assert(m_multisend >= 0);

	cout << "Running as: " << ( m_target_is_ipv6 ? "ipv6" : "ipv4" ) << " to addr " << m_target_addr
		<< " port " << m_target_port << " blocksize " << m_blocksize
		<< " multisend " << m_multisend
		<< endl;
}

void c_ipbench::prepare_socket() {
	_info("Prepare socket");
	if (! m_target_is_ipv6) { // ipv4
		_info("Preparing ipv4");
		m_sock = socket(AF_INET, SOCK_DGRAM, 0);
		if (m_sock<0) error("Can not create socket");
		m_sockaddr4.sin_family = AF_INET;
		m_sockaddr4.sin_port = htons( m_target_port );

		std::cout << "Resolving the address (ipv4)" << std::endl;
		auto host_data = gethostbyname2(m_target_addr.c_str(), AF_INET);
		if (!host_data) throw std::runtime_error("Can not resolve hostname");
		_info("host_data addr: " << (void*)host_data );
		{ // convert
			_info("host_data h_length: " << host_data->h_length );
			unsigned char *addr_as_bytes = reinterpret_cast<unsigned char*>( & m_sockaddr4.sin_addr.s_addr);
			assert( sizeof(m_sockaddr4.sin_addr.s_addr) == host_data->h_length); // do both have same number of bytes
			assert( sizeof(m_sockaddr4.sin_addr.s_addr) == 4); // just a sanity check

			cout << "Addr as bytes: " ;
			for (int i=0; i<host_data->h_length; ++i)
				cout << static_cast<unsigned int>(static_cast<unsigned char>( host_data->h_addr[i] )) << " ";
			cout << endl;
			_info("Will write to " << (void*)addr_as_bytes);
			for (int i=0; i<host_data->h_length; ++i) addr_as_bytes[i] = host_data->h_addr[i]; // TODO
			_info("Address parsed");
		}
	}
	else { // ipv6
		_info("Preparing ipv6");
		m_sock = socket(AF_INET6, SOCK_DGRAM, 0);
		if (m_sock<0) error("Can not create socket");
		m_sockaddr6.sin6_family = AF_INET6;
		m_sockaddr6.sin6_port = htons( m_target_port );

		std::cout << "Resolving the address '" << m_target_addr << "' as ipv6..." << std::endl;
		auto host_data = gethostbyname2(m_target_addr.c_str(), AF_INET6);
		for (int i=0; i<host_data->h_length; ++i) m_sockaddr6.sin6_addr.s6_addr[i] = host_data->h_addr[i];
			_info("Address parsed");
	}
	_info("Prepare socket - done");
}

typedef unsigned short int bufix_t; // buf index

void encrypt_buffer(vector<unsigned char> & buffer) {
	for (bufix_t i=0; i<buffer.size(); ++i) buffer[i] = buffer[i] ^ xorpass;
}

void c_ipbench::event_loop() {
	_info("Entering event loop");

	typedef vector<unsigned char> t_buffer;
	vector<t_buffer> buffer_aggr( m_multisend ); // our buffers to aggregate and multi-send

	for (int i=0; i<m_multisend; ++i) buffer_aggr.at(i) = t_buffer(m_blocksize, static_cast<unsigned char>(222));

	c_counter counter    (std::chrono::seconds(1),true);
	c_counter counter_big(std::chrono::seconds(3),false);


	long int loop_nr=0;
	long int aggr_ix=0; // the aggr-buffer currently to use
	while (1) {
		++loop_nr;

		// _info("Working on aggregated buffer, number: aggr_ix="<<aggr_ix);
		t_buffer buffer = buffer_aggr[ aggr_ix ];
		aggr_ix++;

		ssize_t sent=0;

		if (encryption) for (size_t i=0; i<buffer.size(); ++i) buffer[i] = 222;

		if (true) {
			buffer.at(0)=100;
			buffer.at(1)=101;
			buffer.at(2)=102;
			int i=3;
			buffer.at(i+0) = loop_nr %256;
			buffer.at(i+1) = (loop_nr >> 8) %256;
			buffer.at(i+2) = (loop_nr >> 16) %256;
			buffer.at(i+3) = (loop_nr >> 24) %256;

			i=buffer.size()-10;
			buffer.at(i)='X';
			i++;
			buffer.at(i+0) = loop_nr %256;
			buffer.at(i+1) = (loop_nr >> 8) %256;
			buffer.at(i+2) = (loop_nr >> 16) %256;
			buffer.at(i+3) = (loop_nr >> 24) %256;

			buffer.at(buffer.size()-1)='E';
		}

		if (encryption) encrypt_buffer(buffer);

		if (m_target_is_ipv6) { // ipv6

			bool aggr_send_now = ( aggr_ix == buffer_aggr.size() );

			if (aggr_send_now) {

				sockaddr_in6 * sockaddr6_ptr = & m_sockaddr6.get();
				sockaddr * addr_ptr = reinterpret_cast<sockaddr*>(sockaddr6_ptr); // guaranteed by Linux. TODO

				const int msghdr_tabsize = 512;
				int msghdr_size = aggr_ix ;
				struct mmsghdr tab_mmsghdr[msghdr_tabsize];

				// http://man7.org/linux/man-pages/man2/readv.2.html
				iovec iovec_tab[msghdr_tabsize][1];

				for (int ai=0; ai<aggr_ix; ++ai) { // aggregate msg that we now process - ai (aggr. i)
					struct msghdr & a_msghdr = tab_mmsghdr[ai].msg_hdr; // multiheader of this aggr. msg

					std::memset(&a_msghdr, 0, sizeof a_msghdr); // zero
					a_msghdr.msg_name = addr_ptr;
					a_msghdr.msg_namelen = sizeof(m_sockaddr6);
						iovec_tab[ai][0].iov_base = static_cast<void*>(buffer.data());
						iovec_tab[ai][0].iov_len = buffer.size();
					a_msghdr.msg_iov = & iovec_tab[ai][0];
					a_msghdr.msg_iovlen = 1;
				}

				int sendmmsg_err = sendmmsg(m_sock, &tab_mmsghdr[0], msghdr_size, 0);
				// _info("sendmmsg_err = " << sendmmsg_err);
				if (sendmmsg_err<0) error("Sent failed");

				for (int i=0; i<sendmmsg_err; ++i) {
					auto len = tab_mmsghdr[i].msg_len ;
					// _info("msg i="<<i<<" len="<<len);
					sent += len ;
				}
				// _info("at loop="<<loop_nr<<" aggr_ix(incremented)="<<aggr_ix<<" Totall send in all msg now: " << sent << " in messages: " << sendmmsg_err);

				counter.tick(sent, std::cout);
				counter_big.tick(sent, std::cout);


				aggr_ix=0;
			} // if now sending the aggr table

				// sent = sendmsg(m_sock, static_cast<void*>(buffer.data()), buffer.size(),  0, addr_ptr,	sizeof(m_sockaddr6));

		} else { // ipv4
			sockaddr_in * sockaddr_ptr = & m_sockaddr4.get();
			sockaddr * addr_ptr = reinterpret_cast<sockaddr*>(sockaddr_ptr); // guaranteed by Linux. TODO
			_info("TODO not ready!"); throw std::runtime_error("Not ready code here");
			sent = sendto(m_sock, static_cast<void*>(buffer.data()), buffer.size(),  0, addr_ptr,	sizeof(m_sockaddr4));
			if (sent<0) error("Sent failed");
		}

		if ( 0 == (loop_nr % (10*1000)) ) _info("loop_nr="<<loop_nr/1000<<"K");

		if (loop_nr > (global_config_end_after_packet + 1000)) { _info("Limit - ending test after loop_nr="<<loop_nr); break; }
	}
}

void c_ipbench::run() {
	std::cout << "Starting tests" << std::endl;
	prepare_socket();
	event_loop();
}

// ------------------------------------------------------------------


int main(int argc, char **argv) {
	std::cerr << disclaimer << std::endl;
	c_ipbench bench;
	vector <string> args;
	for (int i=0; i<argc; ++i) args.push_back(argv[i]);
	bench.configure(args);
	bench.run();
}


