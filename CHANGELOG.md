# Changelog

All notable changes to the RVBBIT Arsenal project will be documented in this file.

## [1.0.0] - 2026-04-20

### Added
- **Attack Module (Project RVBBIT)**
  - Full DKOM-based hiding of processes, files, and kernel module.
  - Syscall hooks for `kill`, `getdents64`, `openat`, `bpf`.
  - eBPF program load blocking.
  - Embedded XMRig miner with CPU throttling and hidden execution.
  - Autonomous SSH worm scanning local subnets and brute-forcing credentials.
  - Anti-detection monitor pausing activity when analysis tools are detected.
  - Persistence via systemd service and modules-load.d configuration.
  - Dropper with automatic privilege escalation (CVE-2025-32463) and user-space persistence.
- **Defense Module (RvbbitSafe)**
  - Kernel detector module identifying DKOM-hidden processes and syscall table tampering.
  - Automatic restoration of original syscall pointers.
  - User-space daemon terminating hidden XMRig processes and removing persistence artifacts.
  - Full cleanup of module files, systemd services, and temporary artifacts.
- **Documentation**
  - Complete technical whitepaper in `docs/whitepaper.md`.
  - README files for both attack and defense components.
- **Security**
  - Virtualization detection prevents execution in sandboxed environments.

### Notes
- The XMRig payload must be embedded manually by the user for legal and ethical reasons.
- This release is intended exclusively for educational and defensive research in controlled laboratory settings.