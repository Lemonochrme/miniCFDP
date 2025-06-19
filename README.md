# miniCFDP

CCSDS File Delivery Protocol C Portable Implementation

Based on CCSDS 720.2-G-4 accessible here https://ccsds.org/Pubs/720x2g4.pdf

## Features

- Sends all the files in a monitored directory following CFDP Metadata -> Data -> EOF protocol

TODO : Review CFDP PDUs frames

## Get started

```sh
make
make run
```

Results

```
# tree cfdp_*
cfdp_outbox
├── cam0.jpg
└── cam1.jpg
cfdp_received

1 directory, 2 files
# tree cfdp_*
cfdp_outbox
├── cam0.jpg
└── cam1.jpg
cfdp_received
├── cam0.jpg
└── cam1.jpg

2 directories, 4 files
```

## CFDP Principles
### Overview

![image](https://github.com/user-attachments/assets/79fd2363-de9e-4bca-a7da-2b82b434b859)
*The CFDP Operates over a Wide Range of Underlying Protocols Source: From CFDP Green Book Part 1 (720x1g4)*

![Diagramme sans nom drawio (1)](https://github.com/user-attachments/assets/7e8bc6f8-d305-4598-bf7d-01712f83343f)
*CFDP Architecture*

- CFDP User : The CFDP entity operates at the request of the CFDP user through service primitives. CFDP User = Software task.
- CFDP Entity : implement the transfer procedures defined by the stanard
- File Storage : Local storage interfaced through an agnostic "Virtual Filestore"

![image](https://github.com/user-attachments/assets/9dc2975d-29a3-423d-ab1d-dece77cac5ec)
*Exemple of Virtual Filestore interface architecture Source: From CFDP Green Book Part 1 (720x1g4)*

The Virtual Filestore features a set of minimal capacilities :
- Create a file
- Delete a file
- Deny a file
- Rename a file
- Append a file
- Replace a file
- Create a directory
- Delete a directory
- Deny a directory
- List the directories

The Management Information Base (MIB) provide the CFDP entity with static context informations needed to perform transfers (e.g. address mapping, communication timer settings for retry)

### Operations

2 types of primitives :
- Requests : The user send request service primitive to the local CFDP entity
- Indications : The local CFDP entity send indications back to notify user



List of request primitives :
- Put : Initiate a CFDP transaction (send a file from local to remote filestore). Parameters : destination entity ID, source and destination file names, messages to user and filestore request. If optional paramters are left blank : default values from the MIB.
- Cancel : Immediate stopping of the transaction followed by notification of users (both source and destination) with Transaction-Finished
- Suspend : Suspends originating entity, if it is issued by the FDU source entity : notify user with Suspended notification
- Resume : Same as above, if it is issued by the FDP source entity : notify user with Resumed notification
- Report : Report the status of an ongoing transaction : basically the local user ask the local CFDP entity to return a report

List of indication primitives :
- Transaction
- Metadata-Recv
- File-Segment-Recv
- Suspended
- Resumed
- EOF-Sent
- Transaction-Finished
- Report
- Fault
- Abandoned
- EOF-Recv

Put request primitive operations :
- Source CFDP Entity sends Metadata (e.g. user messages and filestore requests) to destination
- Source CFDP Entity sends file data to destination
- Upon reception of the metadata PDU the CFDP destination entity creates and initializes the datastructures it will use to track the transaction, remember any filestore request and notify remote CFDP user (i.e. its local user) : Metadata-Recv
- Upon reception of EOF PDI the CFDP destination entity notify user by sending EOF-Recv
- If entire FDU transferred : destination entity executes the filestore operations.





