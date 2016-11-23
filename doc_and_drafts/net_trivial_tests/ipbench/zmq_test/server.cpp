//
//  Hello World server in C++
//  Binds REP socket to tcp://*:5555
//  Expects "Hello" from client, replies with "World"
//
#include <zmq.hpp>
#include <string>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <chrono>

int main () {
        typedef std::chrono::high_resolution_clock Time;
        typedef std::chrono::milliseconds ms;
  const char * protocol =
    "tcp://*:5555";
    //  Prepare our context and socket
  zmq::context_t context (1);
  zmq::socket_t socket (context, ZMQ_PUB);
  //    socket.setsockopt (ZMQ_RA`TE, &rate, sizeof (rate));
  //socket.connect ("epgm://eth0;239.192.1.1:5556");
  //socket.bind("epgm://eth0;239.192.1.1:5556");
  socket.bind(protocol);
  unsigned long data_counter = 0;
  unsigned long counter = 0;
  const int data_size = 100;
  auto t0 = Time::now();
  while (counter < 15*1000*1000) {
        unsigned char data[data_size];
        for (int i=0; i<data_size; i++) data[i] = 222;
    zmq::message_t request((void*)data, data_size, NULL);
    data_counter += data_size;
    counter++;
    socket.send(request);
  }
  auto t1 = Time::now();
  auto duration = t1-t0;
  ms d = std::chrono::duration_cast<ms>(duration);
  std::cout << "packets send: " << counter << ", data send: " << data_counter << " B in : " << d.count() << " ms" << std::endl;
  std::cout << "kpck/s : " << counter/d.count() << ", Mbit/s : " << ((data_counter/(d.count()/1000.))/(1024.*1024.))*8 << ", MB/s : " << (data_counter/(d.count()/1000.))/(1024.*1024.) << std::endl;
  return 0;
}
