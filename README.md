# miniCFDP

CCSDS File Delivery Protocol C Portable Implementation

Based on CCSDS 720.2-G-4

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

## CFDP Protocol Data Units Overview

