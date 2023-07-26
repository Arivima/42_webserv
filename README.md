
```diff
- Disclaimer!
+ On this github only the project is final
! Project research, documentation and other supporting documents are work in progress. 
```
</br>
</br>
</br>

# 42_webserv

<i>Simple HTTP server made from scratch</i>
description of project

## Features of this Webserv
- Sockets programming : Basic server-client communication (using the <sys/socket.h> lib)
- Sockets programming : IO multiplexing with epoll suite, sockets are non blocking
- Web server architecture : based on NGINX architecture (worker process) -> Server data structure, configuration data structure
- Web server architecture : one worker (one process) is handling all connections using epoll, 
- Configuration files : inspired by Nginx configuration file, see details for directives below
- Configuration directives : see "./_webserv/configuration_files/README.md"
- HTTP protocol : 1.1
- HTTP methods : GET, POST, DELETE
- HTTP Redirections
- HTTP error codes : see "./webserv_dictionnary.md"
- File upload :
- File download : 
- data type handled : plain/text, binary
- Chunked encoding
- dynamic content handled with CGI, CGI scripts in python and perl
- Timers set to 60 seconds default
- HTML scripts, CSS
- uses stream objects instead of file descriptors for most read/write operation except for sockets
- Error management, leaks, siege

host, listen, server_name, location, method, root, index, autoindex, body_size, error_page, return, exec_cgi, extension_cgi)


## Content of this github
### _webserv
This folder contains the final and complete version of our Webser, pushed and validated 18/07/23.

### ./webserv_*
We are aiming to finalize the following documents in the coming weeks, currently they are still work in progress :
- ./webserv_dictionnary.md
- ./webserv_research.pdf
- ./webserv_flowchart.pdf
- ./webserv_correction_prep.md
- ./_webserv/configuration_files/README.md

<i> <b>WIP</b> Research is available at :</i>
https://docs.google.com/document/d/1i5g1XgHDpUf_c7DF9Sp-EdEj2Bdzipvbjh13hLPty0w/edit?usp=sharing
disclaimer : this was our research document during the project and hasn´t been finalized, it's a bit of a mess but there are resourceful information

## How to use this Webserv

..TBD

### Default Functioning of Our Webserver

The default configuration file is located at `configuration_files/default.conf`.</br>
The root is always at `var/www`.</br>
This folder contains all the static files that our webserver can return to the client (html, css, images and even javascript files!).</br>
The index page is located at `/html/landing_page.html`.</br>

...TBC
