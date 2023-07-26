#!/usr/bin/python3

import threading
import requests
import time

url = "http://localhost:8080/test_post/Huge_file"
file_path = "/nfs/homes/mmarinel/Desktop/42Projects/42_webserv/testing/python/output.txt"

file_content=b''
def make_post_request():
	try:
		session = requests.Session()
		headers = {
			"Content-Type": "text/plain"
		}
		for _ in range(num_requests):
			response = session.post(url, data=file_content, headers=headers)
			print(f"Thread {threading.current_thread().name}: Response Status Code - {response.status_code}")

		# session.close()
	except Exception:
		pass


# Get the number of threads from the user
num_threads = int(input("Enter the number of threads: "))

# Get the number of requests per thread from the user
num_requests = int(input("Enter the number of requests per thread: "))

# Open the file
file = open(file_path, "rb")
file_content = file.read()
file.close()
# Create and start the threads
threads = []
for i in range(num_threads):

	thread = threading.Thread(target=make_post_request, name=f"Thread-{i+1}")
	thread.start()
	threads.append(thread)
	time.sleep(0.2)


# Wait for all threads to complete
for thread in threads:
	thread.join()

print("All threads completed.")
