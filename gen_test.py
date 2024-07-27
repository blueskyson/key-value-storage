import os
import random
import string
import subprocess

# Define the maximum value for an unsigned long long in Python
ULLONG_MAX = 2**64 - 1

# Global dictionary to store keys and values
data_dict = {}

def generate_random_string(length):
    characters = string.ascii_letters  # English characters (both uppercase and lowercase)
    return ''.join(random.choices(characters, k=length))

def gen_put():
    lines = 210000
    random.seed(1)  # Set seed for reproducibility

    with open("put.input", 'wb') as fp:        
        for _ in range(1, lines):
            key = random.randint(0, ULLONG_MAX)
            random_string = generate_random_string(random.randint(0, 128))
            v = random_string.ljust(128, '-')
            l = f"P {key} {v}\n"
            data_dict[key] = v
            fp.write(l.encode())

def gen_get():
    with open("get.input", 'wb') as fp:
        for key in data_dict.keys():
            l = f"G {key}\n"
            fp.write(l.encode())

def check_correctness():
    lineNum = 0
    with open("out.output", 'r') as fp:
        lines = fp.read().splitlines()
        for line in lines:
            lineNum += 1
            parts = line.split(' ', 1)
            key = int(parts[0])
            value = parts[1]
            if data_dict[key] != value:
                print("Error in line:" + str(lineNum) + " Key: " + str(key) + " Expected Value: " + data_dict[key])
                exit()
def main():
    gen_put()
    gen_get()

    cwd = os.getcwd()
    subprocess.run([cwd + "/main", "put.input", "out.output"])
    subprocess.run([cwd + "/main", "get.input", "out.output"])
    check_correctness()
    print("pass")

if __name__ == "__main__":
    main()