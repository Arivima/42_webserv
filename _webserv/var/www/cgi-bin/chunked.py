#!/usr/bin/python

import sys

# Read the input from stdin
input_data = sys.stdin.read()

# Set the response headers
print("HTTP/1.1 200 OK\r\n")
print("Content-Type: text/plain\r\n")
print("\r\n")
# Send the response with the input content
print(input_data)