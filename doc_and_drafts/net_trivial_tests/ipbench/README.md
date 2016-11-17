
How to use this tests:

```

1)
In console 1
./go-tun

-> and we should get card like:
galaxy0   Link encap:UNSPEC  HWaddr 00-00-00-00-00-00-00-00-00-00-00-00-00-00-00-00
          inet6 addr: fd00:808:808:808:808:808:808:808/8 Scope:Global
          UP POINTOPOINT RUNNING NOARP MULTICAST  MTU:1500  Metric:1
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
ping will NOT WORK since no peering works, but then console1 will print the ping ICMP6 raw data + TUN/TAP/ethernet? headers)

-> and then console 1 shows data receive speed: (tun)
-> while console 2 shows send speed of sending:

Window 10.000s:   119.800 Kpck/s ,  29984.732 Mib/s  =  3748.092 MiB/s ; 
Window 10.000s:    59.900 Kpck/s ,  29933.548 Mib/s  =  3741.693 MiB/s ; 


TODO:
increase speed by MTU 1500 to 9000 and more?



```

