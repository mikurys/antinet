

@100 packets: 2.5 Gbps
17.800GiB; Speed: 3822.365 Kpck/s ,  2916.233 Mib/s  = 364.529 MiB/s ; Window 10.000s: 2643.726 Kpck/s ,  2017.003 Mib/s  = 252.125 MiB/s ;

@1000 packets: 14 Gbps
30.000GiB; Speed: 1949.146 Kpck/s ,  14870.800 Mib/s  = 1858.850 MiB/s ; Window 16.526s: 1949.145 Kpck/s ,  14870.800 Mib/s  = 1858.850 MiB/s ;

<<<<<<< HEAD
@65K packets: 110 Gbps
Packet size: 64996 B gives 100.000GiB; Speed: 217.957 Kpck/s ,  108087.116 Mib/s  = 13510.889 MiB/s ; Window 7.579s: 217.957 Kpck/s ,  108087.050 Mib/s  = 13510.881 MiB/s ; 

@9K packets: 60 Gbps
Packet size: 8996 B gives 100.000GiB; Speed: 893.294 Kpck/s ,  61337.627 Mib/s  = 7667.203 MiB/s ; Window 13.356s: 893.294 Kpck/s ,  61337.622 Mib/s  = 7667.203 MiB/s ; 

@9K 1mt: 75 Gbps (more accurate timer?)
Packet size: 9000 B gives: 100.000GiB; Speed: 1096.316 Kpck/s ,  75278.033 Mib/s  = 9409.754 MiB/s ; Window 10.882s: 1096.316 Kpck/s ,  75278.027 Mib/s  = 9409.753 MiB/s ; 

@9K 2mt: 88 Gbps (small increase for 2 thread...)
Packet size: 9000 B gives: 100.000GiB; Speed: 1286.213 Kpck/s ,  88317.261 Mib/s  = 11039.658 MiB/s ; Window 9.276s: 1286.213 Kpck/s ,  88317.253 Mib/s  = 11039.657 MiB/s ; 

@65K 2mt: 213 Gpbs (~ +98% increase for 2 thread)
79.302GiB; Speed: 430.298 Kpck/s ,  213389.484 Mib/s  = 26673.685 MiB/s ; Window 1.007s: 447.015 Kpck/s ,  221679.415 Mib/s  = 27709.927 MiB/s ;

@9K 2mt: 96 Gpbs (better increase 2 thread when less locking on stats)
this report does NOT add the two of per-thread counters, so it shows half speed here:
info: a.cpp+230 Exiting thread #1
100.000GiB; Speed: 736.125 Kpck/s ,  50545.703 Mib/s  = 6318.213 MiB/s ; Window 16.207s: 736.125 Kpck/s ,  50545.699 Mib/s  = 6318.212 MiB/s ; 
96.057GiB; Speed: 669.976 Kpck/s ,  46003.626 Mib/s  = 5750.453 MiB/s ; Window 1.001s: 1048.868 Kpck/s ,  72020.069 Mib/s  = 9002.509 MiB/s ; 
info: a.cpp+216 mt#0 will exit, since we processed all=107374185000 B.
info: a.cpp+230 Exiting thread #0
100.000GiB; Speed: 680.093 Kpck/s ,  46698.297 Mib/s  = 5837.287 MiB/s ; Window 17.542s: 680.093 Kpck/s ,  46698.296 Mib/s  = 5837.287 MiB/s ; 
=======
@65K packets: 14 Gbps
17.202GiB; Speed: 1846.478 Kpck/s ,  14087.509 Mib/s  = 1760.939 MiB/s ; Window 10.003s: 1846.480 Kpck/s ,  14087.525 Mib/s  = 1760.941 MiB/s ;

model name      : Intel(R) Core(TM) i3-2100 CPU @ 3.10GHz
debian 3.16.0-4-amd64 #1 SMP Debian 3.16.36-1+deb8u2 (2016-10-19) x86_64 GNU/Linux
>>>>>>> shmem: pattern check; 167 Gbps @65K (+data) (intel i3)

@100 packets: 4.5 Gbps
28.386GiB; Speed: 6095.051 Kpck/s ,  4650.155 Mib/s  = 581.269 MiB/s ; Window 10.001s: 6138.362 Kpck/s ,  4683.199 Mib/s  = 585.400 MiB/s ;

@1000 packets: 30.5 Gbps
22.603GiB; Speed: 4041.145 Kpck/s ,  30831.493 Mib/s  = 3853.937 MiB/s ; Window 1.000s: 4009.270 Kpck/s ,  30588.305 Mib/s  = 3823.538 MiB/s ;

@65K packets: 167 Gbps (on begining 197 Gbps, latter 145 Gbps)
204.006GiB; Speed: 336.580 Kpck/s ,  166913.696 Mib/s  = 20864.212 MiB/s ; Window 10.012s: 336.580 Kpck/s ,  166913.746 Mib/s  = 20864.218 MiB/s ;
