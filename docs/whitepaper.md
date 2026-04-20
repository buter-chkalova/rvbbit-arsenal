# RVBBIT Arsenal: Anatomy of a Perfect Rootkit and Its Ultimate Defeat

## Abstract

This document provides a complete technical analysis of Project RVBBIT—a next-generation Linux kernel rootkit—and RvbbitSafe, the countermeasure designed to neutralize it. We detail DKOM-based hiding, eBPF bypass, autonomous worm propagation, and the defense mechanisms that restore system integrity.

## 1. Introduction

Project RVBBIT represents the current pinnacle of offensive Linux rootkit technology. It combines direct kernel object manipulation (DKOM), system call table hooking, eBPF program load blocking, an embedded cryptocurrency miner, and an autonomous SSH worm. RvbbitSafe is the corresponding defensive framework that detects hidden processes, restores hooked system calls, terminates the miner, and removes all persistence mechanisms.

This whitepaper serves as both a technical deep-dive and an educational resource for security researchers, penetration testers, and defenders seeking to understand modern kernel-level threats.

## 2. Attack Component: Project RVBBIT

### 2.1 Kernel-Level Stealth

RVBBIT employs DKOM to hide its presence:
- **Module hiding**: Removes itself from the kernel's module list and sysfs, masquerading as the `acpi` driver.
- **Process hiding**: Delists the miner process from the task list and PID structures, making it invisible to `ps`, `top`, and `/proc`.
- **File hiding**: Hooks `getdents64` and `openat` to filter directory listings and file access attempts for files matching a randomly generated prefix.
- **Network hiding**: Hooks `tcp4_seq_show` to conceal connections to the mining pool (port 3333) and SSH (port 22).

### 2.2 System Call Hooking

The rootkit locates the `sys_call_table` via `kallsyms` or brute-force memory scanning. It then temporarily disables write protection (`write_cr0`) to overwrite the following entries:
- `sys_kill` – used to receive magic signals for hiding/pausing the miner.
- `sys_getdents64` – used to hide files.
- `sys_openat` – used to deny access to hidden files.
- `sys_bpf` – used to block eBPF program loads, neutralizing modern detection tools.

### 2.3 eBPF Bypass

By hooking `sys_bpf` and returning `-EPERM` for `BPF_PROG_LOAD` commands, RVBBIT prevents security solutions from attaching eBPF probes that could detect its activities.

### 2.4 Embedded XMRig Miner

The static XMRig binary is embedded as a C array within the kernel module. Upon loading, the rootkit writes the binary to a temporary file with a hidden name and executes it with CPU throttling and low priority. The miner is hidden via DKOM and restarted by a watchdog if it crashes.

### 2.5 Autonomous Network Worm

RVBBIT scans all active network interfaces, identifies local subnets, and probes for open SSH ports (22). It then performs a dictionary attack using a built-in list of common credentials. Upon successful authentication, it downloads and executes the dropper on the victim machine, enabling self-propagation.

### 2.6 Anti-Detection Monitor

A kernel workqueue periodically scans for the presence of forensic and monitoring tools (e.g., `top`, `strace`, `tcpdump`, `rkhunter`). If any are detected, the miner is temporarily killed and network activity is paused until the tools are no longer present.

### 2.7 Persistence

The rootkit installs itself via:
- `/etc/modules-load.d/acpi.conf` – ensures the module loads at boot.
- A systemd service (`rvbbit-helper.service`) that loads the module if it is not already present.

### 2.8 Dropper and Privilege Escalation

The dropper (`rvbbit_installer`) is a static ELF binary containing the kernel module as an embedded array. It attempts to:
1. Load the module directly if running as root.
2. Exploit CVE-2025-32463 (sudo chroot) to gain root privileges.
3. Install user-space persistence in `.bashrc`, `.profile`, and `/etc/profile` if privilege escalation fails.

## 3. Defense Component: RvbbitSafe

### 3.1 Kernel Detector

The `detector.ko` module performs the following actions:
- Scans the process list and identifies hidden PIDs by comparing the task list with the PID hash table.
- Verifies the integrity of the `sys_call_table` by comparing current pointers with the original kernel symbols.
- Restores any hooked system calls by rewriting the original addresses.

### 3.2 User-Space Daemon

The `rvbbitsafe` daemon runs continuously and:
- Scans `/proc` for processes with `xmrig` in their command line and terminates them.
- Removes persistence artifacts, including the kernel module file, `modules-load.d` configuration, and systemd service.
- Cleans up temporary files created by the rootkit.

### 3.3 Deployment

The defense components are installed via a single script (`install.sh`) that compiles and loads the detector module and starts the daemon.

## 4. Conclusion

The RVBBIT Arsenal demonstrates the full lifecycle of a modern Linux rootkit—from infection and stealth to propagation and monetization—and provides a complete, effective countermeasure. This duality makes it an invaluable resource for understanding and defending against advanced kernel-level threats.

## 5. Legal and Ethical Disclaimer

This software is provided **exclusively** for authorized educational and defensive research. The authors unequivocally condemn any malicious use. Unauthorized deployment of this software may violate computer crime laws and result in severe legal penalties. The authors assume no liability for improper use.