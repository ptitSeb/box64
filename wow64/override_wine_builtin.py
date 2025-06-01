#!/usr/bin/env python3

import subprocess
import sys

data = 'Wine builtin DLL\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'

def main(output_file):

    with open(output_file, 'r+b') as f:
        f.seek(64)  # Move cursor to DOS stub
        f.write(data.encode('utf-8'))

main(sys.argv[1])