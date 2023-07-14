#!/usr/bin/python3

import sys
import os

# Read the input from stdin
def read_from_stdin_until_eof():
	input_data = ""
	while True:
		chunk = sys.stdin.read()
		if not chunk:
			return (input_data)
		input_data += chunk
		if len(input_data) > int(os.environ.get("MAX_BODY_SIZE")):
			raise ValueError("Content too large")

try:
	input_data = read_from_stdin_until_eof()
	# Set the response headers
	print("HTTP/1.1 200 OK")
	print("Content-Type: text/plain")
	print("Content-Length: " + str(len(input_data)))
	print("")
	# Send the response with the input content
	sys.stdout.write(input_data)
except ValueError:
	err_code = 413
	msg = "Content Too Large"
	html = (
	f"<!DOCTYPE html>" +
	f"<html lang=\"en\">" +
	f"<head>"+
	f"	<meta charset=\"UTF-8\" />"+
	f"	<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />"+
	f"	<title>{err_code} {msg}</title>"+
	f"</head>"+
	f"<body>"+
	f"	<center>"+
	f"		<h1>Webserv PiouPiou</h1>"+
	f"		<h2>{err_code} {msg}</h2>"+
	f"	</center>"+
	f"</body>"+
	f"</html>"
	)
	print("HTTP/1.1 " + str(err_code) + " " + msg)
	print("Content-Type: text/html")
	print("Content-Length: " + str(len(html)))
	print("")
	print(html)