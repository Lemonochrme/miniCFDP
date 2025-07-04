# Minimal CFDP Class 1 Implementation

This project provides a portable, minimal implementation of CCSDS CFDP (Class 1) in ANSI C.
It is statically allocated, platform-independent, and compiles under both Linux and RTEMS.

## Features
- Fully static memory (no malloc)
- Modular architecture: Core, FS, Comm, MIB, UI
- Initial support for Class 1 (unacknowledged mode) - Ongoing

## ToDo

- PDU Generation
    - Fixed Header Serialization
    - Metadata PDU generation
    - File Data PDU generation
    - EOF PDU generation
- File Data Segmentation
- Think about how to handle both standard and qualified CNES libc libraries

## Building
Run `make` on a POSIX system.

## Directory Structure
```
├── Makefile                        # Compilation and Testing
├── src
│   ├── comm                        # Communication Interface (abstraction of the Underlying Transport (UT) network layer used to send and receive CFDP PDUs)
│   │   ├── cfdp_comm.h
│   │   ├── cfdp_comm_pus.c
│   │   └── cfdp_comm_udp.c
│   ├── core                        # CFDP Core (implementing CFDP Class 1 logic)
│   │   ├── cfdp_core.c             # State machines
│   │   ├── cfdp_core.h
│   │   ├── cfdp_pdu.c              # PDU serialization and deserialization
│   │   └── cfdp_pdu.h
│   ├── fs                          # File Store Interface (abstraction of the underlying file system)
│   │   ├── cfdp_fs.h
│   │   ├── cfdp_fs_posix.c
│   │   └── cfdp_fs_rtems.c
│   ├── mib                         # Management Information Base (holds the configurations and state parameters for the CFDP entity)
│   │   ├── cfdp_mib.c
│   │   └── cfdp_mib.h
│   └── ui                          # CFDP User Interface APIs
│       ├── cfdp_user.c
│       └── cfdp_user.h
```

### Testing

Run all tests using:

```bash
make test.all
```