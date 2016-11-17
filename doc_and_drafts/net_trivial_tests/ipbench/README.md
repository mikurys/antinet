
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

-> and then console 1 shows data receive speed:

Window 10.000s:   182.700 Kpck/s ,   1466.373 Mib/s  =   183.297 MiB/s ;
 9.033GiB; Speed:   170.741 Kpck/s ,   1370.386 Mib/s  =   171.298 MiB/s ; Window 2.000s:   173.000 Kpck/s ,   1388.519 Mib/s  =   173.565 MiB/s ; 

while console 2 shows send speed of sending
(they match speed)

Window 10.000s:   182.700 Kpck/s ,   1393.890 Mib/s  =   174.236 MiB/s ; 
 8.591GiB; Speed:   177.385 Kpck/s ,   1353.337 Mib/s  =   169.167 MiB/s ; Window 2.000s:   173.000 Kpck/s ,   1319.885 Mib/s  =   164.986 MiB/s ; 
 8.920GiB; Speed:   177.370 Kpck/s ,   1353.229 Mib/s  =   169.154 MiB/s ; Window 2.000s:   177.000 Kpck/s ,   1350.403 Mib/s  =   168.800 MiB/s ; 
 9.274GiB; Speed:   177.821 Kpck/s ,   1356.670 Mib/s  =   169.584 MiB/s ; Window 2.000s:   190.000 Kpck/s ,   1449.585 Mib/s  =   181.198 MiB/s ; 
Error: Sent failed


TODO:
increase speed by MTU 1500 to 9000 and more?



```

