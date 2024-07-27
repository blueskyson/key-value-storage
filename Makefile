# Define variables
CC = gcc
CFLAGS = -g
OUTPUT = main
SRC = main.c lib/*.c
PYTHON_SCRIPT = gen_test.py

# Default target
all: build
	python $(PYTHON_SCRIPT)

# Build target
build:
	$(CC) $(CFLAGS) -o $(OUTPUT) $(SRC)

# Clean target (optional)
clean:
	rm -f $(OUTPUT) put.input get.input out.output