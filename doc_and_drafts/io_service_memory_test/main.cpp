#include<iostream>
#include <cstdlib>
#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>

int main()
{
	size_t count;
	std::cin >> count;
	std::cout << "Creating " << count << " io_services ..." << std::endl;
	for(size_t i=0; i<count; i++)
		boost::asio::io_service io_service;
}
