# Key Value Storage

## Overview

Key Value Storage is an assignment of *CSIE3006 - Operating System* by [Hung-Chang Hsiao](https://www.csie.ncku.edu.tw/en/members/27) at National Cheng Kung University. In this assignment, I made a single-threaded key value storage in C++. In this assignment, I referenced famous key-value storage projects such as [LevelDB](https://en.wikipedia.org/wiki/LevelDB) and other search and storage algorithms, and utilized [Bloom filter](https://en.wikipedia.org/wiki/Bloom_filter), [skip list](https://en.wikipedia.org/wiki/Skip_list), [sorted string tables (SSTable)](https://www.scylladb.com/glossary/sstable/), and [binary search](https://en.wikipedia.org/wiki/Binary_search) to enhance the speed of database searches. As a result, I achieved the second-highest grade in this assignment.

Now, I am rewriting this assignment in [object-oriented techniques in C](https://dmitryfrank.com/articles/oop_in_c), ensuring clean code and adhering to SOLID principles. There are still a few memory leaks detected in this program, and I am currently working on fixing them.

Website: [https://jacklinweb.github.io/posts/key-value-storage/](https://jacklinweb.github.io/posts/key-value-storage/)

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