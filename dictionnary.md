# PROJECT DICTIONNARY
Content:
1. Directives implemented  
2. CGI protocol's Environment variables  
3. HTTP Status codes  
3.1. Overall codes used in this project  
3.2. specific to method GET  
3.3.specific to method DELETE  
3.4.specific to method POST  
4. Sys call Error codes  
4.1. stat()  
4.2. opendir()  
4.3. rmdir()  


# Directives implemented
-> refer to README.md in ./configuration_files

# CGI protocol's Environment variables
env_type : char** env

| i | field | description | 
| - | :-----| :-----------|
| 23 | REQUEST_URI | Holds the original URI (Uniform Resource Identifier) sent by the client, including the path, query parameters, and fragments |
| 24 | SCRIPT_NAME | Represents the path of the CGI script being executed, relative to the web server's document root |
| 17 | QUERY_STRING | Contains the query parameters passed in the URL after the "?" character |
| 15 | PATH_INFO | Optionally contains extra path information from the HTTP request that invoked the script, specifying a path to be interpreted by the CGI script. PATH_INFO identifies the resource or sub-resource to be returned by the CGI script, and it is derived from the portion of the URI path following the script name but preceding any query data. |
| 16 | PATH_TRANSLATED | Represents the translated file path corresponding to the PATH_INFO
| 1 | AUTH_TYPE | This variable contains the authentication type used for the current request, typically provided by the web server |
| 2 | CONTENT_LENGTH | This variable holds the size, in bytes, of the request body (the content being sent in the request) |
| 3 | CONTENT_TYPE | The MIME type of the body of the request, or null if the type is not known. For HTTP servlets, the value returned is the same as the value of the CGI variable CONTENT_TYPE. |
| 4 | GATEWAY_INTERFACE | The revision of the CGI specification being used by the server to communicate with the script. It is "CGI/1.1". |
| 5 | HTTP_ACCEPT | Variables with names beginning with "HTTP_" contain values from the request header, if the scheme used is HTTP. HTTP_ACCEPT specifies the content types your browser supports. For example, text/xml. |
| 6 | HTTP_ACCEPT_CHARSET |     Character preference information. Used to indicate the client's prefered character set if any. For example, utf-8;q=0.5. |
| 7 | HTTP_ACCEPT_ENCODING | Defines the type of encoding that may be carried out on content returned to the client. For example, compress;q=0.5. |
| 8 | HTTP_ACCEPT_LANGUAGE | Used to define which languages you would prefer to receive content in. For example, en;q=0.5. If nothing is returned, no language preference is indicated. |
| 9 | HTTP_COOKIE | Cookie String.
| 10 | HTTP_FORWARDED | If the request was forwarded, shows the address and port through of the proxy server. |
| 11 | HTTP_HOST | Specifies the Internet host and port number of the resource being requested. Required for all HTTP/1.1 requests. |
| 12 | HTTP_PROXY_AUTHORIZATION | Used by a client to identify itself (or its user) to a proxy which requires authentication. |
| 13 | HTTP_USER_AGENT | The type and version of the browser the client is using to send the request. For example, Mozilla/1.5. |
| 14 | NCHOME | The NCHOME environment variable.
| 18 | REMOTE_ADDR | This variable holds the IP address of the client making the request |
| 19 | REMOTE_HOST | The hostname making the request. If the server does not have this information, it should set REMOTE_ADDR and leave this unset. |
| 20 | REMOTE_IDENT | If the HTTP server supports RFC 931 identification, then this variable will be set to the remote user name retrieved from the server. Usage of this variable should be limited to logging only. |
| 21 | REMOTE_USER | Returns the login of the user making this request if the user has been authenticated, or null if the user has not been authenticated. |
| 22 | REQUEST_METHOD | Returns the name of the HTTP method with which this request was made. For example, GET, POST, or PUT. |
| 25 | SERVER_NAME | Contains the domain name or IP address of the server handling the request |
| 26 | SERVER_PORT | Holds the port number on which the server is listening for requests, typically 80 for HTTP and 443 for HTTPS |
| 27 | SERVER_PROTOCOL | This variable specifies the name and version of the protocol used by the server to handle the request, such as "HTTP/1 |
| 28 | SERVER_SOFTWARE | Provides information about the web server software being used, including its name and version |
| 29 | WEBTOP_USER | The user name of the user who is logged in. |


# HTTP Status codes
https://developer.mozilla.org/en-US/docs/Web/HTTP/Status#client_error_responses  
## Overall codes used in this project
### 2** Successful responses
| code | field | description | 
| - | :-----| :-----------|
| 200 | OK |		request was successful |
| 201 | Created |	 |
| 204 | No Content |request was successful -> no additional content to send back in the response body |
### 3** Redirection messages 
| code | field | description | 
| - | :-----| :-----------|
| 307 | Temporary Redirect | The server sends this response to direct the client to get the requested resource at another URI with the same method that was used in the prior request. This has the same semantics as the 302 Found HTTP response code, with the exception that the user agent must not change the HTTP method used: if a POST was used in the first request, a POST must be used in the second request.|
| 308 | Permanent Redirect | This means that the resource is now permanently located at another URI, specified by the Location: HTTP Response header. This has the same semantics as the 301 Moved Permanently HTTP response code, with the exception that the user agent must not change the HTTP method used: if a POST was used in the first request, a POST must be used in the second request. |

### 4** Client error responses 
| code | field | description | 
| - | :-----| :-----------|
| 400 | Bad Request | - The server cannot or will not process the request due to something that is perceived to be a client error (e.g., malformed request syntax, invalid request message framing, or deceptive request routing). |
| 403 | Forbidden | - The client does not have access rights to the content; that is, it is unauthorized, so the server is refusing to give the requested resource. Unlike 401 Unauthorized, the client's identity is known to the server. |
| 404 | Not Found | - The server cannot find the requested resource. In the browser, this means the URL is not recognized.  |
| 405 | Method Not Allowed | - The HyperText Transfer Protocol (HTTP) 405 Method Not Allowed response status code indicates that the server knows the request method, but the target resource doesn't support this method. |
| 409 | Conflict | - The deletion could not be completed due to a conflict with the current state of the resource.  |
| 413 | Content Too Large | - The HTTP 413 Content Too Large response status code indicates that the request entity is larger than limits defined by server; the server might close the connection or return a Retry-After header field. |
| 414 | URI Too Long | - The URI requested by the client is longer than the server is willing to interpret. |
### 5** Server error responses
| code | field | description | 
| - | :-----| :-----------|
| 500 | Internal Server Error | - An unexpected error occurred on the server while processing the deletion request. |
| 501 | Not Implemented | - The HyperText Transfer Protocol (HTTP) 501 Not Implemented server error response code means that the server does not support the functionality required to fulfill the request. |
| 502 | Bad Gateway | - The HyperText Transfer Protocol (HTTP) 502 Bad Gateway server error response code indicates that the server, while acting as a gateway or proxy, received an invalid response from the upstream server. |
| 504 | Gateway Timeout | - The HyperText Transfer Protocol (HTTP) 504 Gateway Timeout server error response code indicates that the server, while acting as a gateway or proxy  |
	
## specific to method GET
### Successful request
### Failed request
## specific to method DELETE
### Successful request
| code | field | description | 
| - | :-----| :-----------|
| 200 | OK | if the response includes an entity describing the status |
| 202 | Accepted | if the action has not yet been enacted |
| 204 | No Content | if the action has been enacted but the response does not include an entity. |
### Failed request
| code | field | description | 
| - | :-----| :-----------|
| 400 | Bad Request | The request could not be understood or was malformed. The server should include information in the response body or headers about the nature of the error.
| 403 | Forbidden | The server understood the request, but the client does not have permission to access the requested resource.
| 404 | Not Found | The requested resource could not be found on the server.
| 414 | URI Too Long  | The URI requested by the client is longer than the server is willing to interpret.
| 409 | Conflict | The request could not be completed due to a conflict with the current state of the resource. This is often used for data validation errors or when trying to create a resource that already exists.
| 500 | Internal Server Error | An unexpected error occurred on the server, indicating a problem with the server's configuration or processing of the request.
## specific to method POST
The action performed by the POST method might not result in a resource that can be 
identified by a URI. In this case, either 200 (OK) or 204 (No Content) 
is the appropriate response status, depending on whether or not the response 
includes an entity that describes the result.
If a resource has been created on the origin server, the response 
SHOULD be 201 (Created) and contain an entity which describes the status of 
the request and refers to the new resource, and a Location header (see section 14.30).
### Successful request
| code | field | description | 
| - | :-----| :-----------|
| 200 | OK |		request was successful -> response body contains result or representation of requested resource |
| 201 | Created |	if the resource has been created on the origin server. response should include a Location header with url of the created resource |
| 204 | No Content |request was successful -> no additional content to send back in the response body |
### Failed request
| code | field | description | 
| - | :-----| :-----------|
| 400 | Bad Request | The request could not be understood or was malformed. The server should include information in the response body or headers about the nature of the error. |
| 403 | Forbidden | The server understood the request, but the client does not have permission to access the requested resource. |
| 404 | Not Found | The requested resource could not be found on the server. |
| 409 | Conflict | The request could not be completed due to a conflict with the current state of the resource. This is often used for data validation errors or when trying to create a resource that already exists. |
| 500 | Internal Server Error | An unexpected error occurred on the server, indicating a problem with the server's configuration or processing of the request. |

# Sys call Error codes

### stat()
| code | description | 
| - | :-----------|
| EACCES| Search permission is denied for one of the directories in the path prefix of path. (See also path_resolution(7).)
| EBADF| fd is bad.
| EFAULT| Bad address.
| ELOOP| Too many symbolic links encountered while traversing the path.
| ENAMETOOLONG|	path is too long.
| ENOENT| A component of path does not exist, or path is an empty string.
| ENOMEM| Out of memory (i.e., kernel memory).
| ENOTDIR| A component of the path prefix of path is not a directory.
| EOVERFLOW| path or fd refers to a file whose size, inode number, or number of blocks cannot be represented in,respectively, the types off_t, ino_t, or blkcnt_t. This error can occur when, for example, an application compiled on a 32-bit platform without -D_FILE_OFFSET_BITS=64 calls stat() on a file whose size exceeds (1<<31)-1 bytes.

### opendir()
| code | description | 
| - | :-----------|
| EACCES | Permission denied. |
| EBADF  | fd is not a valid file descriptor opened for reading. |
| EMFILE | The per-process limit on the number of open file descriptors has been reached. |
| ENFILE | The system-wide limit on the total number of open files has been reached. |
| ENOENT | Directory does not exist, or name is an empty string. |
| ENOMEM | Insufficient memory to complete the operation. |
| ENOTDIR | name is not a directory. |

### rmdir()
| code | description | 
| - | :-----------|
| EACCES| Write access to the directory containing pathname was not allowed, or one of the directories in the path prefix of pathname did not allow search permission.  (See also path_resolution(7).) |
| EBUSY | pathname is currently in use by the system or some process that prevents its removal.  On Linux, this means pathname is currently used as a mount point or is the root directory of the calling process. |
| EFAULT| pathname points outside your accessible address space. |
| EINVAL| pathname has .  as last component. |
| ELOOP | Too many symbolic links were encountered in resolving pathname. |
| ENAMETOOLONG | pathname was too long. |
| ENOENT| A directory component in pathname does not exist or is a dangling symbolic link. |
| ENOMEM| Insufficient kernel memory was available. |
| ENOTDIR| pathname, or a component used as a directory in pathname, is not, in fact, a directory. |
| ENOTEMPTY | pathname contains entries other than . and .. ; or, pathname has ..  as its final component. POSIX.1 also allows EEXIST for this condition. |
| EPERM | The directory containing pathname has the sticky bit (S_ISVTX) set and the process's effective user ID is neither the user ID of the file to be deleted nor that of the directory containing it, and the process is not privileged (Linux: does not have the CAP_FOWNER capability).
| EPERM | The filesystem containing pathname does not support the removal of directories. |
| EROFS | pathname refers to a directory on a read-only filesystem. |

