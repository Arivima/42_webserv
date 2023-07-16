#!/usr/bin/python3

import random
import string

file_size = 1 * 1024 * 1024 * 1024  # 1 GB

# Open the file in write mode
with open("output.txt", "w") as file:
    while file.tell() < file_size:
        # Generate a random string of length 1024
        data = ''.join(random.choice(string.ascii_letters + string.digits) for _ in range(1024))

        # Write the generated string to the file
        file.write(data)

print("File generation completed.")
