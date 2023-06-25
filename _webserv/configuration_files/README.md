CONFIGURATION DOCUMENTATION

List of implemented directives, within their relevant scope
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

Configuration requirements:
host : specifies a specific interface (IP) the server should listens on,
if not specified server listens on all interfaces available on the host
listen : mandatory
server_name : mandatory for servers that share same interface(IP)+Port, optional otherwise.
location
method : if not specified, all subject requested methods allowed (POST, GET, DELETE)
root : if not specified, root is empty (= project folder)
index : if not specified and autoindex not on, return default error page
autoindex : if not specified, considered off
body_size : if not specified, body_size is unlimited
error_page : if not specified, default error page
return
exec_cgi
extension_cgi

List of implemented directives, single/multi values
| name | single | multi | value_type | value | value_ex | value_default |
| ----- | :-----:| :----:| :---------:| :----| :--------| :--------|
| host | Y | . | str | IP | localhost, 192.168.1.14 | localhost |
| listen | Y | . | int | port | 8080 |
| server_name | . | Y | str | domain | example.com |
| location | Y | . | str | rel_path | /images/ |
| method | . | Y | str | http method | GET, POST, DELETE |
| root | Y | . | str | f_sys_path | /var/www/html|
| index | . | Y | str | rel_path | index.html |
| autoindex | Y | . | bool | | on/off |
| body_size | Y | . | int | | 4096 |
| error_page | . | Y | int[...int]/str | http errcode + rel_path | 400 404 /404.html |
| return | Y | . | int/str | http redircode + str/rel_path | 301 http://example.com/new-url |
| exec_cgi | Y | . |
| extension_cgi | Y | . |

note :

- rel_path is a path relative to the fs root set in the "root" directive

Writing a configuration file:

- keep indentation for each block
