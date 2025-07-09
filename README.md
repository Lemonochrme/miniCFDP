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
    - Finished PDU
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

# Terms and Definitions

- LV : Lenght-Value
    - Lenght is a coded value indicating how many bytes are next to represent the value
    - Value is the real data of variable lenght

Example : Let the file `"input.txt"` (9 characters) to represent it in a Lenght-Value manner we would encode it like so :

```
             Value
   |------------------------|
09 69 6E 70 75 74 2E 74 78 74
Λ
Lenght
```

In the case of the CCSDS CFDP the format of LV objects is defined in the following :


| Field  | Size (bit) | Values   | Lenght of value  |
| ------ | ---------- | -------- | ---------------- |
| Length | 8          | 0 to 255 | Length of value  |
| Value  | 8 x Length |          |                  |


- TLV : Type-Lenght-Value


| Field  | Size (bit) | Values         | Lenght of value |
| ------ | ---------- | -------------- | --------------- |
| Type   | 8          | Directive Code | Nature of value |
| Length | 8          | 0 to 255       | Length of value |
| Value  | 8 x Length |                |                 |

| Directive Code (Hexadecimal) | Directive      |
| ---------------------------- | -------------- |
| 00                           | Reserved       |
| 01                           | Reserved       |
| 02                           | Reserved       |
| 03                           | Reserved       |
| 04                           | EOF PDU        |
| 05                           | Finished PDU   |
| 06                           | ACK PDU        |
| 07                           | Metadata PDU   |
| 08                           | NAK PDU        |
| 09                           | Prompt PDU     |
| 0C                           | Keep Alive PDU |
| 0D-FF                        | Reserved       |

