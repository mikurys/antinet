#include <zmq.hpp>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <chrono>

int main (void)
{
        typedef std::chrono::high_resolution_clock Time;
        typedef std::chrono::milliseconds ms;
  zmq::context_t context(1);
  const char * protocol =
    "ipc://localhost:5555";
  //  Socket to talk to server
  printf ("Connecting to server...");
  zmq::socket_t sock (context, ZMQ_SUB);
  //sock.bind(protocol);
  sock.connect(protocol);
  sock.setsockopt (ZMQ_SUBSCRIBE, "", 0);
  printf ("done. \n");

  unsigned long data_counter = 0;
  unsigned long counter = 0;
  auto t0 = Time::now();
  while(true){
    zmq::message_t reply;
    sock.recv (&reply);
    if(counter ==0)
        t0 = Time::now();
    data_counter += reply.size();
    if(strcmp((char *)reply.data(), "end") == 0)
        break;
    counter++;
  }
  sock.close();
  auto t1 = Time::now();
  auto duration = t1-t0;
  ms d = std::chrono::duration_cast<ms>(duration);
  std::cout << "packets read: " << counter << ", data read: " << data_counter << " B in : " << d.count() << " ms" << std::endl;
  std::cout << "kpck/s : " << counter/d.count() << ", Mbit/s : " << ((data_counter/(d.count()/1000.))/(1024.*1024.))*8 << ", MB/s : " << (data_counter/(d.count()/1000.))/(1024.*1024.) << std::endl;
  return 0;
}
