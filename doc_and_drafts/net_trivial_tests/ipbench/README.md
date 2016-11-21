
How to use this tests:

** some data here sometimes could be not 100% correct, check sources and run yourself to confirm **
- errata:  in previous commit, the mtu was increased while this doc here shown old ifconfig info (wrong)
- errata:  in previous versions, the counter/stats was not exact: look at the window (1s, 3s window etc) reading only.

```


CURRENT RESULT:
(on comp Dragon: AMD FX(tm)-6300 Six-Core Processor , Debian 8)

30.0 Gb/s for 65KB packets is the goal - if not crypto
30.0 Gb/s for 65KB packets is the goal - if crypto (4 or 8 core good CPU [1])
 1.0 Gb/s for 1KB packets is the goal, since even loopback UDP sending of such small packets is slow

30.0 Gb/s for 65KB packets, data written from ipclient to ipv6 (going to TUN)
 1.3 Gb/s for  1KB packets ... as above ...
30.0 Gb/s for 65KB packets, data read from /dev/tun inside tunserver
 1.3 Gb/s for  1KB packets ... as above ... around 0.1% are dropped unless txqueuelen>2000 (see below)
data seems correct (checking magic bytes at begin/end)



MTU / buffer size:
for 65K packets:
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


*** also *** to avoid losing 0.1% of packets as LOST/dropped,
after starting the go-tun, run as root command:
ifconfig galaxy0 txqueuelen 5000


2a)
In console 2
./go-cli-big   # to run test on 65K sized packages

-> and then console 1 shows data receive speed: (tun)
-> while console 2 shows send speed of sending:

after the test, when tunserver.elf waits for ENTER to quit, ifconfig of card can look like:
galaxy0   Link encap:UNSPEC  HWaddr 00-00-00-00-00-00-00-00-00-00-00-00-00-00-00-00
          inet6 addr: fd00:808:808:808:808:808:808:808/8 Scope:Global
          UP POINTOPOINT RUNNING NOARP MULTICAST  MTU:65500  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:1997530 errors:0 dropped:2971 overruns:0 carrier:0 <------------------
          collisions:0 txqueuelen:500
          RX bytes:0 (0.0 B)  TX bytes:3092176440 (2.8 GiB)

In tun:
 2.759GiB; Speed:   146.846 Kpck/s ,   1738.779 Mib/s  =   217.347 MiB/s ; Window 1.000s:   152.000 Kpck/s ,   1799.805 Mib/s  =   224.976 MiB/s ; 
 Window 3.000s:   159.667 Kpck/s ,   1890.584 Mib/s  =   236.323 MiB/s ; 
 Packets: uniq=1909K ; Max=1911470 Dupli=0 Reord=0 Missing(now)=2470 0.13% LOST-PACKETS 


2b) *** this results are if edited to send 65000 packets ***
In console 2
./go-cli-big

60.584GiB; Speed:    58.824 Kpck/s ,  29194.551 Mib/s  =  3649.319 MiB/s ; Window 2.000s:    64.500 Kpck/s ,  32011.826 Mib/s  =  4001.478 MiB/s ; 
60.536GiB; Speed:    62.500 Kpck/s ,  30994.415 Mib/s  =  3874.302 MiB/s ; Window 2.000s:    64.500 Kpck/s ,  31986.237 Mib/s  =  3998.280 MiB/s ; 


2x)

easy test:
ping6 fd00:808:808:808:808:808:808:1111
for basic test.
ping will NOT WORK since no peering works, but then console1 will print the ping ICMP6 raw data + TUN/TAP/ethernet? headers




Also some additional debug could be seen (maybe)
e.g.:
info: /home/rafalcode/work/antinet/doc_and_drafts/net_trivial_tests/ipbench/tunserver.cpp+353 size_read=65052 start_pos=52
0 0 134 221 96 0 0 0 253 240 17 64 253 0 8 8 8 8 8 8 8 8 8 8 8 8 8 8 253 0 8 8 8 8 8 8 8 8 8 8 8 8 17 17 169 193 33 107 253 240 155 203 100 101 102 5 0 0 0 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 222 
info: /home/rafalcode/work/antinet/doc_and_drafts/net_trivial_tests/ipbench/tunserver.cpp+353 size_read=65052 start_pos=52


[1] crypto speeds done with ./tunserver.elf --demo crypto_stream_bench given around 1 GB = 8 Gbps (on 1 core)


TODO:
[x] increase speed by MTU 1500 to 9000 and more? -- yes, big packets go fast and no lost
[x] increase card txqueue to remove the 0.1% dropped at 1K size of packets



```

