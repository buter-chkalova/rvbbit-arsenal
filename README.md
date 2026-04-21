# RVBBIT Arsenal: The Ultimate Offense & Defense Duality

![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)
![Platform: Linux x86_64](https://img.shields.io/badge/Platform-Linux%20x86__64-blue)
![Status: Research Epic](https://img.shields.io/badge/Status-Research%20Epic-red)

## Overview

This repository houses two sides of the same coin:

- **`attack/`** – Project RVBBIT, a classic Linux kernel rootkit with embedded miner and network worm.
- **`defense/`** – RvbbitSafe, the definitive countermeasure that detects, neutralizes, and removes every trace of RVBBIT.

Together, they form the comprehensive educational resource on modern Linux rootkit warfare.

## Quick Start

### Build the Nightmare (Attack)
```bash
cd attack
First, embed the XMRig payload (see attack/README.md)
./build.sh
Deploy rvbbit_installer on isolated test VM

Note: Detailed instructions on embedding payloads are in attack/README.md.
Deploy the Cure (Defense)
bash
cd defense
chmod +x install.sh
sudo ./install.sh
Documentation
Full technical whitepaper available in docs/whitepaper.md.

Disclaimer
This software is provided for authorized educational and defensive research only. Misuse is strictly prohibited and may violate computer crime laws. The authors assume no liability for improper use.

## Limitations

This is a learning prototype. It uses dated techniques, generates detectable telemetry, and will not evade modern EDRs or hypervisor integrity checks. Not intended for operational use.