# RvbbitSafe – The Cure

## Overview

RvbbitSafe is the definitive countermeasure against the Project RVBBIT rootkit. It consists of:

- **Kernel detector module (`detector.ko`)** – scans for hidden processes and restores hooked system calls.
- **User-space daemon (`rvbbitsafe`)** – terminates hidden miner processes and removes persistence artifacts.

## Deployment

Run the installation script from within a **Linux environment** (native or WSL2) with root privileges:

```bash
chmod +x install.sh
sudo ./install.sh

# The script will:

1. Compile and load the kernel detector module.

2. Compile and launch the user-space daemon in the background.

## Verification

Check kernel logs for detector output: dmesg | grep RvbbitSafe

Verify that the daemon is running: ps aux | grep rvbbitsafe

## Removal
To stop the daemon and unload the detector:

bash
sudo pkill rvbbitsafe
sudo rmmod detector

## Disclaimer
This software is provided for authorized educational and defensive research only. Misuse is strictly prohibited.