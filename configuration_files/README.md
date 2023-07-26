//!UPDATE the directives

# CONFIGURATION DOCUMENTATION

## Formatting Rules:

- opening block brackets must be at the same line as the block level (i.e.: "server \{")
- closing block brackets must be on their own line (i.e.: "\<something\>\}" is not valid)
- only CGI interpreters pathname can be considered absolute (i.e.: not relative to the project's folder)

If one of those rules is not respected, config file is considered non-valid.

# List of implemented directives, within their relevant scope
name | root | http | server | location |
| --------- | :----:| :-----:| :-----:| -----: |
host | - | - | Y | -
listen | - | - | Y | - |
server_name | - | - | Y | - |
location | - | - | - | Y |
method | - | - | Y | Y |
root | - | - | Y | Y |
index | - | - | Y | Y |
autoindex | - | - | Y | Y |
body_size | - | - | Y | Y |
error_page | - | - | Y | Y |
return | - | - | Y | Y |
exec_cgi | - | - | Y | Y |
extension_cgi | - | - | Y | Y |

note:

- if a directive is declared outside of its scope, configuration file is invalid
- location is a made-up directive that exists in the location block and points to the location path
  - we also have a made-up location directive in the server, with key location<location_path> used for checking duplicate locations in a server block


### List of implemented directives, single/multi values
| name | single | multi | value_type | Description | value_ex | value_default |
| ----- | :-----:| :----:| :---------:| :----| :--------| :--------|
| host | Y | . | str | IP | localhost, 192.168.1.14 | localhost |
| listen | Y | . | int | port | 8080 |
| server_name | . | Y | str | domain | example.com |
| location | Y | . | str | rel_path | /images/ |
| method | . | Y | str | http method | GET, POST, DELETE |
| root | Y | . | str | f_sys_path | /var/www/html|
| index | . | Y | str | rel_path | index.html |
| autoindex | Y | . | bool | | on/off |
| body_size | Y | . | int | max_body_size | 5000, 500K, 200M | 1M|
| error_page | . | Y | int[...int]/str | http errcode + rel_path | 400 404 /404.html |
| return | Y | . | int/str | http redircode + str/rel_path | 301 http://example.com/new-url |
| exec_cgi | Y | . |
| extension_cgi | Y | . |

note :

- rel_path is a path relative to the fs root set in the "root" directive

## Directives Specification
- host : *\<IP\>*. Specifies a specific network interface (IP) the server should listen on; if not specified, the server will listen on all available local interfaces
- listen : *\<Port\>*. Specifies the port the server will listen on --**mandatory**
- server_name : *\<String\>*. Specifies the virtual hostname for this server --**mandatory** for servers that share the same physical medium (i.e.: same interface and port), optional otherwise.
- location : *\<path\>*. Specifies the path indicated by the location block where this directive appears on
- method : *\<[GET | POST | DELETE]\>*. Specifies the http methods allowed at the current block route. **GET is always allowed by default**.
- root : *\<path\>*. Pathname relative to the server's project folder where files should be searched from. if not specified, root is empty (= project folder)
- index : *\<pathname\>.html*. The page to return when uri of a GET request points to a directory. if not specified and autoindex not on, return default error page
- autoindex : *\<[on | off]\>*. When index directive does not apply for current GET request and autoindex is on, return a directory listing of the requested folder.
- body_size : *\<positive int\>*. Maximum size allowed for client's requests at current block route. If not specified, body_size is unlimited
- error_page : "\<http_error_code\>[...\<http_error_code\>] \<filename\>.html". Indicates the error page to return in case of listed http errors. if not specified, a default error page will be returned.
- return : TODO
- cgi_enable : "\<pathname of interpreter\> \<file_extension\>[...\<file_extension\>] <filename>. Specifies pathname of CGI interpreter for executing scripts having \<file_extension\> as their file extension

## "location" block and "method" directive in Handling of POST and DELETE
Our Handling of the POST and DELETE methods is based entirely on the use of the "location" block for a less vulnerable handling of such methods.</br>
- the request URI must match a location block inside the selected configuration file and this configuration block must have the "method" directive declared with the given request method.
	- any request uri containing ".." will be considered non-valid
- It is highly discouraged to use the "method" directive inside the server block as it will enable the given methods on all routes within that block. That is, creation and deletion at any possible route within the upload_path, if existent, or, even worse, within the root folder otherwise.</br>
It is still possible to use this directive inside the server block when the block has no declared location blocks and we may still want to use the POST or DELETE methods.

By following these guidelines we make it possible for this to happen:
- POST/DELETE on \<not_safe\> with filename \<pippo\> ---> will give an error
- POST/DELETE on \<safe\> with filename \<pippo\> ---> will work




OLD
<!-- ## "location" block and Handling of POST and DELETE
Our Handling of the POST and DELETE methods is based entirely on the use of the "location" block for a less vulnerable handling of such methods.</br>
- We allow POST and DELETE requests only on URIs ending with a folder name
- The name of the actual file to be created or deleted at that location will be given by the provided query string.
	- If no query string is provided, an http error is returned.
	- the name in the query string must be given in the format "name=\<filename\>" and must not contain any "/" character for security concerns, otherwise, an http error will be returned.
- the request URI must match a location block inside the selected configuration file and this configuration block must have the "method" directive declared with the given request method.
- It is highly discouraged to use the "method" directive inside the server block as it will enable the given methods on all routes within that block. That is, creation and deletion at any possible route within the upload_path, if existent, or, even worse, within the root folder otherwise.</br>
It is still possible to use this directive inside the server block when the block has no declared location blocks and we may still want to use the POST or DELETE methods.

By following these guidelines we make it possible for this:
- POST/DELETE on \<not_safe\> with filename \<pippo\> ---> will give an error
- POST/DELETE on \<safe\> with filename \<pippo\> ---> will work -->