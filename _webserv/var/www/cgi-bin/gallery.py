#!/usr/bin/python

import cgi
import os
import shutil

# Set the path to the directory where the uploaded photos will be stored
UPLOAD_DIR = "../gallery/"

# In the configuration file, set up the appropriate authorization
# method POST;
# method DELETE;

# Function to generate the HTML page
def generate_html(photo_files, current_photo):
    # Generate the previous and next links for photo browsing
    prev_link = ""
    next_link = ""
    if len(photo_files) > 1:
        if current_photo > 0:
            prev_link = f'<a href="?photo={current_photo - 1}">&lt; Previous</a>'
        if current_photo < len(photo_files) - 1:
            next_link = f'<a href="?photo={current_photo + 1}">Next &gt;</a>'

    # Generate the HTML page
    html = f'''
    <html>
    <head>
        <title>Photo Viewer</title>
        <style>
            .center {{
                display: flex;
                justify-content: center;
                align-items: center;
                height: 80vh;
            }}
        </style>
    </head>
    <body>
        <div class="center">
            <img src="{photo_files[current_photo]}" style="max-height: 80%; max-width: 80%;">
        </div>
        <div style="text-align: center; margin-top: 20px;">
            <input type="file" name="photo" accept="image/*">
            <input type="submit" value="Upload">
        </div>
        <div style="text-align: center; margin-top: 20px;">
            {prev_link} {next_link}
        </div>
    </body>
    </html>
    '''
    return html

# Function to handle file uploads
def handle_upload(form):
    fileitem = form['photo']
    if fileitem.filename:
        # Create the upload directory if it doesn't exist
        if not os.path.exists(UPLOAD_DIR):
            os.makedirs(UPLOAD_DIR)

        # Save the uploaded file to the upload directory
        filename = os.path.join(UPLOAD_DIR, fileitem.filename)
        with open(filename, 'wb') as f:
            f.write(fileitem.file.read())

# Get the current photo index from the query parameters
query_params = cgi.FieldStorage()
current_photo = int(query_params.getvalue('photo', 0))

# Get the list of existing photo files in the upload directory
photo_files = [f for f in os.listdir(UPLOAD_DIR) if os.path.isfile(os.path.join(UPLOAD_DIR, f))]

# Handle file uploads if a file was submitted
if 'photo' in query_params:
    handle_upload(query_params)

# Generate and print the HTML page
html = generate_html(photo_files, current_photo)
print("Content-type: text/html\n")
print(html)