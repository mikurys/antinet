
How to use this tests:

** some data here sometimes could be not 100% correct, check sources and run yourself to confirm **
- errata:  in previous commit, the mtu was increased while this doc here shown old ifconfig info (wrong)

```


CURRENT RESULT:
30 Gb/s data written from ipclient to ipv6 (going to TUN)
30 Gb/s data read from /dev/tun inside tunserver
data seems correct (checking magic bytes at begin/end)


MTU / buffer size:
MTU on card: 65500
UDP Datagram size in test ipclient.elf program: 65000 - so that the entire UDP with UDP headers,
will not fragment IP packet.

Data position:
in tun raw data, the offset of UDP datagram sent, is +52 octets

1)
In console 1
./go-tun

-> and we should get card like:
galaxy0   Link encap:UNSPEC  HWaddr 00-00-00-00-00-00-00-00-00-00-00-00-00-00-00-00  
          inet6 addr: fd00:808:808:808:808:808:808:808/8 Scope:Global
          UP POINTOPOINT RUNNING NOARP MULTICAST  MTU:65500  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:500 
          RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)



2)
In console 2
./go-cli

(
or
ping6 fd00:808:808:808:808:808:808:1111
for basic test.
ping will NOT WORK since no peering works, but then console1 will print the ping ICMP6 raw data + TUN/TAP/ethernet? headers
)

-> and then console 1 shows data receive speed: (tun)
-> while console 2 shows send speed of sending:

60.584GiB; Speed:    58.824 Kpck/s ,  29194.551 Mib/s  =  3649.319 MiB/s ; Window 2.000s:    64.500 Kpck/s ,  32011.826 Mib/s  =  4001.478 MiB/s ; 
60.536GiB; Speed:    62.500 Kpck/s ,  30994.415 Mib/s  =  3874.302 MiB/s ; Window 2.000s:    64.500 Kpck/s ,  31986.237 Mib/s  =  3998.280 MiB/s ; 




Also some additional debug could be seen (maybe)
e.g.:
info: /home/rafalcode/work/antinet/doc_and_drafts/net_trivial_tests/ipbench/tunserver.cpp+353 size_read=65052 start_pos=52
0 0 134 221 96 0 0 0 253 240 17 64 253 0 8 8 8 8 8 8 8 8 8 8 8 8 8 8 253 0 8 8 8 8 8 8 8 8 8 8 8 8 17 17 169 193 33 107 253 240 155 203 100 101 102 5 0 0 0 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 
info: /home/rafalcode/work/antinet/doc_and_drafts/net_trivial_tests/ipbench/tunserver.cpp+353 size_read=65052 start_pos=52



TODO:
increase speed by MTU 1500 to 9000 and more?



```

