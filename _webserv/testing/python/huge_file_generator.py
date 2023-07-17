#!/usr/bin/python3

import random
import string
import threading
import math

output_file = "output.txt"

# Get the number of threads from the user
num_threads = int(input("Enter the number of threads: "))

# Get the number of requests per thread from the user
file_size = int(input("Enter the desired file size (in MB): "))
file_size = 1 * 1024 * 1024 * file_size  # in bytes

chunk_size = math.floor(float(file_size) / float(num_threads))


threads = []
chunks = [""] * num_threads

def generate_chunk(chunk_size):
    characters = random.choices(string.ascii_letters + string.digits, k=chunk_size)
    return ''.join(characters)

def thread_function(pos):
    # Generate a chunk in the thread
    chunk = generate_chunk(chunk_size)

    # Append the chunk to the thread-safe list
    chunks[pos] = chunk

# Create and start the threads
for i in range(num_threads):
    thread = threading.Thread(target=thread_function, args=(i,))
    thread.start()
    threads.append(thread)

# Wait for all threads to complete
for thread in threads:
    thread.join()

# Combine the chunks in the main thread
with open(output_file, "a") as file:
    for i in range(num_threads):
        file.write(chunks[i])

print("File generation completed.")
