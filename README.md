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

### Testing

Run all tests using:

```bash
make test.all
```

Each test verifies a module of the system:

#### File System (`test_fs`)

* Create a test file.
* Write text to it.
* Read the file back and check the content.
* Rename the file.
* Delete the renamed file.

#### UDP Communication (`test_comm_udp`)

* Start a sender and a receiver as two local processes.
* Initialize each with a different CFDP entity ID and port.
* Sender sends a message to the receiver using CFDP's UDP logic.
* Receiver receives the message and checks:
  * The data matches.
  * The sender entity ID is correct.


#### Core Logic (`test_core`)

* Create a source file with known content.
* Set up local and remote entity info in memory.
* Register a callback to capture transaction events.
* Start a transaction to send the file.
* Verify:
  * The file was fully sent.
  * The completion event was triggered.
* Simulate receiving the same file using `cfdp_process_pdu()`.
* Verify:
  * The file was re-created.
  * The content matches.
  * The receive transaction completed successfully.
* Clean up all files.