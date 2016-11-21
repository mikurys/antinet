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

#include "libs1.hpp"
#include "counter.hpp"
#include "cpputils.hpp"

// linux (and others?) select use:
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>

// for low-level Linux-like systems TUN operations
#include <fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>
// #include <net/if_ether.h> // peer over eth later?
// #include <net/if_media.h> // ?

#include "../NetPlatform.h" // from cjdns

// #include <net/if_tap.h>
#include <linux/if_tun.h>

#include "config.h"

// ------------------------------------------------------------------


// Tweaking the network (e.g. for speeeeed)
const int config_tun_mtu  = 65500;
const int config_buf_size = 65535 * 1;

typedef unsigned int bufix_t; // buf index

static_assert( config_buf_size >= config_tun_mtu , "Buffer should be (I think) not smaller then MTU");
static_assert( std::numeric_limits<bufix_t>::max() >= config_buf_size , "Too small type for buf index." );

// ------------------------------------------------------------------

void error(const std::string & msg) {
	std::cout << "Error: " << msg << std::endl;
	throw std::runtime_error(msg);
}

// ------------------------------------------------------------------

// TODO to lib "ip46"

// sockaddr is the base class for sockaddr_in and for sockaddr_in6
// see also http://stackoverflow.com/a/18579605
static_assert( sizeof(sockaddr) <= sizeof(sockaddr_in) , "Invalid size of ipv4 vs ipv6 addresses" );
static_assert( sizeof(sockaddr) <= sizeof(sockaddr_in6) , "Invalid size of ipv4 vs ipv6 addresses" );

class c_ip46_addr { ///< any address ipv6 or ipv4, in system socket format
	public:
		enum { tag_none, tag_ipv4, tag_ipv6 } m_tag; ///< current type of address

	public:
		c_ip46_addr();

		void set_ip4(sockaddr_in in4);
		void set_ip6(sockaddr_in6 in6);
		sockaddr_in  get_ip4() const;
		sockaddr_in6 get_ip6() const;

		static c_ip46_addr any_on_port(int port); ///< return my address, any IP (e.g. for listening), on given port. it should listen on both ipv4 and 6
		friend ostream &operator << (ostream &out, const c_ip46_addr& addr);

	private:
		struct t_ip_data {
			union { ///< the address is either:
				sockaddr_in in4;
				sockaddr_in6 in6;
			};
		};

		t_ip_data m_ip_data;
};

c_ip46_addr::c_ip46_addr() : m_tag(tag_none) { }

void c_ip46_addr::set_ip4(sockaddr_in in4) {
	_assert(in4.sin_family == AF_INET);
	m_tag = tag_ipv4;
	this->m_ip_data.in4 = in4;
}
void c_ip46_addr::set_ip6(sockaddr_in6 in6) {
	_assert(in6.sin6_family == AF_INET6);
	m_tag = tag_ipv6;
	this->m_ip_data.in6 = in6;
}

sockaddr_in  c_ip46_addr::get_ip4() const {
	_assert(m_tag == tag_ipv4);
	auto ret = this->m_ip_data.in4;
	_assert(ret.sin_family == AF_INET);
	return ret;
}
sockaddr_in6 c_ip46_addr::get_ip6() const {
	_assert(m_tag == tag_ipv6);
	auto ret = this->m_ip_data.in6;
	_assert(ret.sin6_family == AF_INET6);
	return ret;
}

c_ip46_addr c_ip46_addr::any_on_port(int port) { ///< return my address, any IP (e.g. for listening), on given port
	as_zerofill< sockaddr_in > addr_in;
	addr_in.sin_family = AF_INET;
	addr_in.sin_port = htons(port);
	addr_in.sin_addr.s_addr = INADDR_ANY;
	c_ip46_addr ret;
	ret.set_ip4(addr_in);
	return ret;
}

ostream &operator << (ostream &out, const c_ip46_addr& addr) {
	if (addr.m_tag == c_ip46_addr::tag_ipv4) {
		char addr_str[INET_ADDRSTRLEN];
		auto ip4_address = addr.get_ip4();
		inet_ntop(AF_INET, &ip4_address.sin_addr, addr_str, INET_ADDRSTRLEN);
		out << addr_str;
	}
	else if (addr.m_tag == c_ip46_addr::tag_ipv6) {
		char addr_str[INET6_ADDRSTRLEN];
		auto ip6_address = addr.get_ip6();
		inet_ntop(AF_INET6, &ip6_address.sin6_addr, addr_str, INET6_ADDRSTRLEN);
		out << addr_str;
	}
	else {
		out << "none";
	}
	return out;
}

// ------------------------------------------------------------------

// TODO: crypto options here
class c_peering { ///< An (mostly established) connection to peer
	public:
		virtual void dd();

		virtual ~c_peering();

	private:
		c_ip46_addr	m_addr; ///< peer address in socket format

		std::string m_pubkey; ///< his pubkey
		// ... TODO crypto type
};

class c_peering_udp : public c_peering { ///< An established connection to UDP peer
	public:
	private:
};

// ------------------------------------------------------------------

class c_tunserver {
	public:
		c_tunserver();

		void configure(const std::vector<std::string> & args);
		void run();

	protected:
		void prepare_socket(); ///< make sure that the lower level members of handling the socket are ready to run
		void event_loop(); ///< the main loop

		void wait_for_fd_event();

		void configure_add_peer(const c_ip46_addr & addr, const std::string & pubkey);

	private:
		int m_tun_fd; ///< fd of TUN file

		int m_sock_udp; ///< the main network socket (UDP listen, send UDP to each peer)

		fd_set m_fd_set_data; ///< select events e.g. wait for UDP peering or TUN input

		vector<c_peering> m_peer; ///< my peers
};

// ------------------------------------------------------------------

using namespace std; // XXX move to implementations, not to header-files later, if splitting cpp/hpp

c_tunserver::c_tunserver() {
}

void c_tunserver::configure_add_peer(const c_ip46_addr & addr, const std::string & pubkey) {
	// m_peer.push_back( ); // XXX
}

void c_tunserver::configure(const std::vector<std::string> & args) {
	if (args.size()>=2) {

		int i=1;
		if (args.at(i) == "-p") {
	//		configure_add_peer( args.at(i+1) , args.at(i+2) ); // XXX
		}

	}
}

/*
GPL: cjdns LICENCE

void NetPlatform_setMTU(const char* interfaceName,
                        uint32_t mtu)
{
    struct ifreq ifRequest;
    int s = socketForIfName(interfaceName, AF_INET6, &ifRequest);

    ifRequest.ifr_mtu = mtu;
    if (ioctl(s, SIOCSIFMTU, &ifRequest) < 0) {
        close(s);
    }

    close(s);
}
*/

void c_tunserver::prepare_socket() {
	m_tun_fd = open("/dev/net/tun", O_RDWR);
	assert(! (m_tun_fd<0) );

  as_zerofill< ifreq > ifr; // the if request
	ifr.ifr_flags = IFF_TUN ; // || IFF_MULTI_QUEUE;
	strncpy(ifr.ifr_name, "galaxy%d", IFNAMSIZ);
	auto errcode_ioctl =  ioctl(m_tun_fd, TUNSETIFF, (void *)&ifr);
	if (errcode_ioctl < 0)_throw( std::runtime_error("Error in ioctl (creating TUN)")); // TODO

	_mark("Allocated interface:" << ifr.ifr_name);

	uint8_t address[16];
	for (int i=0; i<16; ++i) address[i] = 8;

	// TODO: check if there is no race condition / correct ownership of the tun, that the m_tun_fd opened above is...
	// ...to the device to which we are setting IP address here:
	address[0] = 0xFD;
	address[1] = 0x00;
	NetPlatform_addAddress(ifr.ifr_name, address, 8, Sockaddr_AF_INET6);

/*
	{
    as_zerofill< ifreq > ifRequest;
    ifRequest.ifr_mtu = 1500;
//		strncpy(ifRequest.ifr_name, "galaxy", IFNAMSIZ);
    auto errcode_ioctl = ioctl(m_tun_fd, SIOCSIFMTU, &ifRequest);
		if (errcode_ioctl < 0)_throw( std::runtime_error("Error in ioctl (setting MTU of TUN)")); // TODO
	}

*/
	NetPlatform_setMTU("galaxy0", config_tun_mtu );
	// NetPlatform_setMTU("galaxy0", 1300);

	// create listening socket
	m_sock_udp = socket(AF_INET, SOCK_DGRAM, 0);
	_assert(m_sock_udp >= 0);

	int port = 9042;
	c_ip46_addr address_for_sock = c_ip46_addr::any_on_port(port);

	{
		sockaddr_in addr4 = address_for_sock.get_ip4();
		auto bind_result = bind(m_sock_udp, reinterpret_cast<sockaddr*>(&addr4), sizeof(addr4));  // reinterpret allowed by Linux specs
		_assert( bind_result >= 0 ); // TODO change to except
	}
	_info("Bind done - listening on UDP on: "); // TODO  << address_for_sock
}

void c_tunserver::wait_for_fd_event() { // wait for fd event
	//_info("Selecting");
	// set the wait for read events:
	FD_ZERO(& m_fd_set_data);
	FD_SET(m_sock_udp, &m_fd_set_data);
	FD_SET(m_tun_fd, &m_fd_set_data);

	auto fd_max = std::max(m_tun_fd, m_sock_udp);
	_assert(fd_max < std::numeric_limits<decltype(fd_max)>::max() -1); // to be more safe, <= would be enough too
	_assert(fd_max >= 1);

	auto select_result = select( fd_max+1, &m_fd_set_data, NULL, NULL,NULL); // <--- blocks
	_assert(select_result >= 0);
}

/*
TODO
struct c_packet_stats {
	double 
};
*/


/// Were all packets received in order?
struct c_packet_check {
	c_packet_check(size_t max_packet_index);

	void see_packet(size_t packet_index);

	vector<bool> m_seen; ///< was this packet seen yet
	size_t m_count_dupli;
	size_t m_count_uniq;
	size_t m_count_reord;
	size_t m_max_index;
	bool m_i_thought_lost; ///< we thought packets are lost

	void print() const;
	bool packets_maybe_lost() const; ///< do we think now that some packets were lost?
};

c_packet_check::c_packet_check(size_t max_packet_index)
	: m_seen( max_packet_index , false ), m_max_index(0)
{ }

bool c_packet_check::packets_maybe_lost() const {
	size_t max_reodrder = 1000; // if more packets are out then it's probably lost.
	// do we have packet-index much higher then number of packets recevied at all:
	if (m_max_index > m_count_uniq + max_reodrder) return true;
	return false;
}

void c_packet_check::see_packet(size_t packet_index) {
	if (packet_index < m_max_index) {
		++ m_count_reord;
	}
	m_max_index = std::max( m_max_index , packet_index );

	if (packets_maybe_lost()) m_i_thought_lost=true;

	if (m_seen.at(packet_index)) {
		++ m_count_dupli;
		const size_t warn_max = 100;
		if (m_count_dupli < warn_max)	{
			_info("duplicate at packet_index="<<packet_index);
			print();
		}
		if (m_count_dupli == warn_max)	_info("duplicate at packet_index - will hide further warnings");
	} else { // a not-before-seen packet index
		++ m_count_uniq;
	}
	m_seen.at(packet_index) = true;
}

void c_packet_check::print() const {
	auto missing = m_max_index - m_count_uniq; // mising now. maybe will come in a moment as reordered, or maybe are really lost
	double missing_part = 0;
	if (m_max_index>0) missing_part = (double)missing / m_max_index;
	auto & out = std::cout;
	out << "Packets: uniq="<<m_count_uniq/1000<<"K ; Max="<<m_max_index
		<<" Dupli="<<m_count_dupli
		<<" Reord="<<m_count_reord
		<<" Missing(now)=" << missing << " "
			<< std::setw(3) << std::setprecision(2) << std::fixed << missing_part*100. << "%";

	if (packets_maybe_lost()) out<<" LOST-PACKETS ";
	else if (m_i_thought_lost) out<<" (packet seemed lost in past, but now all looks fine)";

	out<<endl;
}

void c_tunserver::event_loop() {
	_info("Entering the event loop");
	c_counter counter    (std::chrono::seconds(1),true);
	c_counter counter_big(std::chrono::seconds(3),true);
	c_counter counter_all(std::chrono::seconds(999999),true);

	fd_set fd_set_data;

	const int buf_size = config_buf_size;
	unsigned char buf[buf_size];
	const bool dbg_tun_data=1;
	int dbg_tun_data_nr = 0; // how many times we shown it

	c_packet_check packet_check(10*1000*1000);

	bool warned_marker=false; // ever warned about marker yet?

	size_t loop_nr=0;


	while (1) {
			++loop_nr;
//			if (0==(loop_nr % (10*1000))) packet_check.print(); // XXX
	//	wait_for_fd_event();

		ssize_t size_read_tun=0, size_read_udp=0;
		const unsigned char xorpass=42;

		//if (FD_ISSET(m_tun_fd, &m_fd_set_data)) { // data incoming on TUN - send it out to peers
			auto size_read=0;
			size_read += read(m_tun_fd, buf, sizeof(buf)); // read data from TUN

			// for (bufix_t i=52; i<size_read; ++i) buf[i] = buf[i] ^ xorpass ; // "decrypt"

			const int mark1_pos = 52;
			bool mark_ok = true;
			if (!(  (buf[mark1_pos]==100) && (buf[mark1_pos+1]==101) &&  (buf[mark1_pos+2]==102)  )) mark_ok=false;

			{ // validate counter 1
				long int packet_index=0;
				for (int i=0; i<4; ++i) packet_index += static_cast<size_t>(buf[mark1_pos+2+1 +i]) << (8*i);
				// _info("packet_index " << packet_index);

				if (packet_index >= global_config_end_after_packet ) {
						cout << "LIMIT - END " << endl << endl;
					_info("Limit - ending test");
					break ;
				} // <====== RET

				packet_check.see_packet(packet_index);
	//			packet_stats.see_size( size_read ); // TODO
			}
			if ( buf[size_read-10] != 'X') {
				if (!warned_marker) _info("Wrong marker X");
				warned_marker=true;
				mark_ok=false;
			}
			if ( buf[size_read-1] != 'E') {
				if (!warned_marker) _info("Wrong marker E");
				warned_marker=true;
				mark_ok=false;
			}

	//		if (!mark_ok) _info("Packet has not expected UDP data! (wrong data read from TUN?) other then "
	//			"should be sent by our ipclient test program");

			if (dbg_tun_data && dbg_tun_data_nr<5) {
				++dbg_tun_data_nr;
				// _info("Read: " << size_read);
				auto show = std::min(size_read,128); // show the data read, but not more then some part
				auto start_pos=0;
				for (int i=0; i<show; ++i) {
					cout << static_cast<unsigned int>(buf[i]) << ' ';
					if ((buf[i]==100) && (buf[i+1]==101) && (buf[i+2]==102)) start_pos = i;
				}
				_info("size_read=" << size_read << " start_pos=" << start_pos);

				cout << endl << endl;
				// buf[buf_size-1]='\0'; // hack. terminate sting to print it:
				// cout << "Buf=[" << string( reinterpret_cast<char*>(static_cast<unsigned char*>(&buf[0])), size_read) << "] buf_size="<< buf_size << endl;
			}

			size_read_tun += size_read;
		//}
		/*
		else if (FD_ISSET(m_sock_udp, &m_fd_set_data)) { // data incoming on peer (UDP) - will route it or send to our TUN
			sockaddr_in6 from_addr_raw; // the address of sender, raw format
			socklen_t from_addr_raw_size; // the size of sender address

			from_addr_raw_size = sizeof(from_addr_raw); // IN/OUT parameter to recvfrom, sending it for IN to be the address "buffer" size
			auto size_read = recvfrom(m_sock_udp, buf, sizeof(buf), 0, reinterpret_cast<sockaddr*>( & from_addr_raw), & from_addr_raw_size);
			// ^- reinterpret allowed by linux specs (TODO)
			// sockaddr *src_addr, socklen_t *addrlen);
		}
		else _erro("No event selected?!"); // TODO throw
 		*/

		bool printed=false;
		printed = printed || counter.tick(size_read_tun, std::cout);
		bool printed_big = counter_big.tick(size_read_tun, std::cout);
		printed = printed || printed_big;
		if (printed_big) packet_check.print();
		counter_all.tick(size_read_tun, std::cout, true);
	}
	_info("Loop done");

	std::cout << endl << endl;
	counter_all.print(std::cout);
	packet_check.print();
}

void c_tunserver::run() {
	std::cout << "Starting tests" << std::endl;
	prepare_socket();
	event_loop();
}

// ------------------------------------------------------------------


int main(int argc, char **argv) {
	std::cerr << disclaimer << std::endl;

/*	c_ip46_addr addr;
	std::cout << addr << std::endl;
	struct sockaddr_in sa;
	inet_pton(AF_INET, "192.0.2.33", &(sa.sin_addr));
	sa.sin_family = AF_INET;
	addr.set_ip4(sa);
	std::cout << addr << std::endl;

	struct sockaddr_in6 sa6;
	inet_pton(AF_INET6, "fc72:aa65:c5c2:4a2d:54e:7947:b671:e00c", &(sa6.sin6_addr));
	sa6.sin6_family = AF_INET6;
	addr.set_ip6(sa6);
	std::cout << addr << std::endl;
*/
	c_tunserver myserver;
	vector <string> args;
	for (int i=0; i<argc; ++i) args.push_back(argv[i]);
	myserver.configure(args);
	myserver.run();

	std::cout << "All done. Press ENTER to exit (and probably delete the virtual card)" << std::endl;
	string x;
	std::getline(std::cin,x);
}


