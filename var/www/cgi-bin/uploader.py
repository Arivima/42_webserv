#!/usr/bin/python3

# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    uploader.py                                         :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mmarinel <mmarinel@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/07/15 13:46:12 by mmarinel          #+#    #+#              #
#    Updated: 2023/07/15 13:46:13 by mmarinel         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

#################################################################################
#																				#
#	Script functioning:															#
#				this script uploads an image to a gallery;						#
#				the name of the image is expected in the query string			#
#				in the form of ?key=value. A missing query string, or a query	#
#				string having more than one variable, will be considered		#
#				a Bad Request.													#
#																				#
#				This script is thought to to work with chunked requests too.	#
#				So, we read from stdin (which is redirected to a pipe)			#
#				a few characters at a time so to check if the content sent is	#
#				too large before reading all of it (which can lead to slow		#
#				performance and defeats the purpose of having a max body size	#
#				directive.														#
#																				#
#################################################################################


import sys
import os

GALLERY_PATH = "/nfs/homes/mmarinel/Desktop/42Projects/42_webserv/_webserv/var/www/gallery/"

class ContentTooLarge(Exception):
	pass
class BadRequest(Exception):
	pass


# Read the input from stdin
def read_from_stdin_until_eof(chunk_size=1024):
	input_data = b""
	while True:
		chunk = sys.stdin.buffer.read(chunk_size)
		if not chunk:
			return (input_data)
		input_data += chunk
		if len(input_data) > int(os.environ.get("MAX_BODY_SIZE")):
			raise ContentTooLarge()

filename=""
msg=""
err_code=0
try:
	input_data = read_from_stdin_until_eof()
	filename = os.environ.get("QUERY_STRING").split("=")
	if len(filename) == 2:
		filename = GALLERY_PATH + filename[1]
	else:
		filename = ""
		raise (BadRequest())
	with open(filename, "wb") as file:
			file.write(input_data)
	file.close()
	# setting the response success page
	html = (
	f"<!DOCTYPE html>" +
	f"<html lang=\"en\">" +
	f"<head>"+
	f"	<meta charset=\"UTF-8\" />"+
	f"	<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />"+
	f"	<title>201 Created</title>"+
	f"</head>"+
	f"<body>"+
	f"	<center>"+
	f"		<h1>Webserv PiouPiou</h1>"+
	f"		<h2>201 Created</h2>"+
	f"	</center>"+
	f"</body>"+
	f"</html>"
	)
	# Set the response headers
	print("HTTP/1.1 201 Created", end="\r\n")
	print("Content-Type: text/html", end="\r\n")
	print("Content-Length: " + str(len(html)), end="\r\n")
	print("", end="\r\n")
	# Send the response with the input content
	print(html)
	sys.exit(0)
except BadRequest:
	err_code = 400
	msg = "Bad Request"
except ContentTooLarge:
	err_code = 413
	msg = "Content Too Large"
except (OSError, NameError, IOError, ValueError):
	if os.path.exists(filename):
		os.remove(filename)
	msg = "Internal Server Error"
	err_code = 500

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
print("HTTP/1.1 " + str(err_code) + " " + msg, end="\r\n")
print("Content-Type: text/html", end="\r\n")
print("Content-Length: " + str(len(html)), end="\r\n")
print("", end="\r\n")
print(html)
