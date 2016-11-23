
How to use this tests:

  * install libzmq-dev
  * build using make
  * run client followed by server in other terminal

```
average speed for packets 1000 B (tcp):

packets send: 15000000, data send: 15000000000 B in : 46021 ms
kpck/s : 325, Mbit/s : 2486.71, MB/s : 310.839

packets read: 14991633, data read: 14991633004 B in : 45988 ms
kpck/s : 325, Mbit/s : 2487.11, MB/s : 310.888

average speed for packets 1000 B (ipc):

packets send: 15000000, data send: 15000000000 B in : 57068 ms
kpck/s : 262, Mbit/s : 2005.34, MB/s : 250.668

packets read: 14965080, data read: 14965080004 B in : 56949 ms
kpck/s : 262, Mbit/s : 2004.86, MB/s : 250.607

```
