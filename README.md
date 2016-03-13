# serial-can-dump
dump serial input to CAN bus and vice-versa (on AVR)

## Serial protocol
The serial protocol will be byte-oriented. Logical groups of a CAN message will be aligned to byte borders as possible.
Base- and Extended-format frames will be the same except one bit. Unused base-format bits are set to zero

### Message format
```
ID3 [ID2 ID1 ID0 LEN [DATA ...]]

ID3: ERR RTR  EXT BIT28  BIT27 BIT26  BIT25 BIT24
ID2: BIT23-BIT16
ID1: BIT17-BIT08
ID0: BIT07-BIT00


LEN: uint8_t with length of DATA in bytes
DATA: optional data
```

### Error frames or other errors
```
ID3 = 1<<ERR | error_flags (other bits except ERR are reserved for future use)
```
message ends after the first byte

### Base frames
```
ERR = 0
BITxx = can_identifier<<18
```

### Extended frames
```
ERR = 0
BITxx = can_identifier
```
