#!/bin/bash
set -e
make
insmod detector.ko
gcc -o rvbbitsafe rvbbitsafe_daemon.c
./rvbbitsafe &
echo "[RvbbitSafe] Installed and running."