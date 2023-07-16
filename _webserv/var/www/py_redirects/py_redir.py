#!/usr/bin/python3
import requests

url = "http://localhost:8080/old-page/"
file_path = "/nfs/homes/mmarinel/Pictures/facebook_small.png"

def send_post_request(url, file_path):
    session = requests.Session()

    # Open the file in binary mode
    with open(file_path, "rb") as file:
        # Read the file content
        file_content = file.read()

    # Send the POST request with the file content in the request body
    response = session.post(url, data=file_content)

    # Handle redirection (HTTP 307 and 308)
    while response.status_code in (307, 308) and "Location" in response.headers:
        url = response.headers["Location"]

        # Close the previous connection
        session.close()

        # Open a new session for the next request
        session = requests.Session()

        # Send the POST request with the file content in the request body on a new connection
        response = session.post(url, data=file_content)

    # Print the response status code and content
    print("Response Status Code:", response.status_code)
    print("Response Content:", response.content)

# Call the function to send the POST request
send_post_request(url, file_path)
