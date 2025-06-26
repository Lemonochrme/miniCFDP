# Minimal CFDP Class 1 Implementation

This project provides a portable, minimal implementation of CCSDS CFDP (Class 1) in ANSI C.
It is statically allocated, platform-independent, and compiles under both Linux and RTEMS.

## Features
- Fully static memory (no malloc)
- Modular architecture: Core, FS, Comm, MIB, UI
- Initial support for Class 1 (unacknowledged mode)

## Building
Run `make` on a POSIX system.

## Directory Structure
```
cfdp_project/
├── Makefile
├── README.md
├── src/
│   ├── core/
│   │   ├── cfdp_core.h
│   │   └── cfdp_core.c
│   ├── fs/
│   │   ├── cfdp_fs.h
│   │   └── cfdp_fs_posix.c
│   ├── comm/
│   │   ├── cfdp_comm.h
│   │   └── cfdp_comm_udp.c
│   ├── mib/
│   │   ├── cfdp_mib.h
│   │   └── cfdp_mib.c
│   └── ui/
│       ├── cfdp_user.h
│       └── cfdp_user.c
```