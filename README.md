# Key Value Storage

## Overview

A simple key-value storage program written in C. 

## Prerequisites

- `Linux`
- `gcc`
- `python` (Python 3.x)


## How to Run

### Build and Test

```
make
```

Expected output:

```
gcc -g -o main main.c lib/*.c
python gen_test.py
pass
```

### Build Only

```bash
make build
```

## Explain

The program reads an input file and an output file. Each line of input file should comply the form of
- `P <key> <value>`: `P` stands for PUT, `key` should be an `unsigned long long` integer, and value should be a 128-length string.
- `G <key>`: `G` stands for GET, and `key` should be an `unsigned long long` integer.

Here is an `example.input` file:

```
P 14089154938208861744 gNytkzUuFNKnPEfqbHaTJYLBNWCrJNXWvVVfGLpMUYAYAVjoYzWuSyMvItTOMeIfi---------------------------------------------------------------
P 7254276152544923774 MyqSFEHilakVCvrSsVIFLtSXWAgOkXzsCWvQvaCOrFPHCjeCSWbXdTxNonPjpinXIHpKzfqrPnnLYYwYlubXxAwRYGKxBbJPIwMglcddVpiDgDSElUxRTOGbkfDUE---
P 7195347014358151499 aFuoiSQDhwnftCVRBNBdcgiBnrAnrfmXOLzEOqvtZhgfEWdCDXspBfUfcpFavQPjOEixJiRwYPCQCLqkqbOWLquudqFxnOOUTysjkksQeNeDrlYcjPEVmfFPelQunTL-
G 14089154938208861744
G 7254276152544923774
G 1234567
P 14089154938208861744 aFuoiSQDhwnftCVRBNBdcgiBnrAnrfmXOLzEOqvtZhgfEWdCDXspBfUfcpFavQPjOEixJiRwYPCQCLqkqbOWLquudqFxnOOUTysjkksQeNeDrlYcjPEVmfFPelQunTL-
G 14089154938208861744
```

You can run

```
./main example.input ./main example.output
```

and see the output.

```
14089154938208861744 gNytkzUuFNKnPEfqbHaTJYLBNWCrJNXWvVVfGLpMUYAYAVjoYzWuSyMvItTOMeIfi---------------------------------------------------------------
7254276152544923774 MyqSFEHilakVCvrSsVIFLtSXWAgOkXzsCWvQvaCOrFPHCjeCSWbXdTxNonPjpinXIHpKzfqrPnnLYYwYlubXxAwRYGKxBbJPIwMglcddVpiDgDSElUxRTOGbkfDUE---
1234567 EMPTY
14089154938208861744 aFuoiSQDhwnftCVRBNBdcgiBnrAnrfmXOLzEOqvtZhgfEWdCDXspBfUfcpFavQPjOEixJiRwYPCQCLqkqbOWLquudqFxnOOUTysjkksQeNeDrlYcjPEVmfFPelQunTL-
```