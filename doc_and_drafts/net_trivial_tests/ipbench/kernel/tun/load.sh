#!/bin/bash
sudo modprobe -r tun
sudo dmesg -c

echo "Loading new:"
sudo insmod ./tun.ko
sudo dmesg -c

