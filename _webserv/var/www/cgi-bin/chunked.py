#!/usr/bin/python

import sys

# Read the input from stdin
input_data = sys.stdin.read()

# Set the response headers
print("HTTP/1.1 200 OK")
print("Content-Type: text/plain")
print("Content-Length: " + str(len(input_data)))
print("")
# Send the response with the input content
sys.stdout.write(input_data)