#ifndef CGI_HPP
# define CGI_HPP

# include <map>
# include <string>
# include <sstream>
// # include <utility>

# include "Webserv.hpp"



// // create a function that stores all env var in a char** env
// 1. AUTH_TYPE: This variable contains the authentication type used for the current request, typically provided by the web server. For example, if the server requires Basic Authentication, AUTH_TYPE will be set to "Basic".
// 2. CONTENT_LENGTH: This variable holds the size, in bytes, of the request body (the content being sent in the request). It is applicable for POST and PUT requests where data is included in the body.
// 3. CONTENT_TYPE: It specifies the MIME type of the request body. For example, if the request contains JSON data, CONTENT_TYPE will be set to "application/json".
// 4. GATEWAY_INTERFACE: This variable specifies the CGI specification version supported by the server. It typically has a value like "CGI/1.1".
// 5. PATH_INFO: PATH_INFO provides additional path information after the script name in the URL. It can be used to pass extra information to the CGI script, such as parameters or file names.
// 6. PATH_TRANSLATED: PATH_TRANSLATED represents the translated file path corresponding to the PATH_INFO. It is often used to locate resources or files based on the information provided in PATH_INFO.
// 7. QUERY_STRING: QUERY_STRING contains the query parameters passed in the URL after the "?" character. It is useful for extracting and processing GET request parameters.
// 8. REMOTE_ADDR: This variable holds the IP address of the client making the request. It can be used to identify the client's location or perform IP-based filtering.
// 9. REMOTE_IDENT: REMOTE_IDENT is an optional variable that may contain the identity information of the client making the request. It is typically used with identd, an obsolete identification protocol.
// 10. REMOTE_USER: If the server performs authentication, REMOTE_USER contains the authenticated username of the client.
// 11. REQUEST_METHOD: REQUEST_METHOD specifies the HTTP method used in the request, such as "GET", "POST", "PUT", "DELETE", etc. It helps the CGI script determine how to process the request.
// 12. REQUEST_URI: REQUEST_URI holds the original URI (Uniform Resource Identifier) sent by the client, including the path, query parameters, and fragments.
// 13. SCRIPT_NAME: SCRIPT_NAME represents the path of the CGI script being executed, relative to the web server's document root. It can be used to locate and identify the script.
// 14. SERVER_NAME: SERVER_NAME contains the domain name or IP address of the server handling the request. It is used to identify the server within a virtual hosting environment.
// 15. SERVER_PORT: SERVER_PORT holds the port number on which the server is listening for requests, typically 80 for HTTP and 443 for HTTPS.
// 16. SERVER_PROTOCOL: This variable specifies the name and version of the protocol used by the server to handle the request, such as "HTTP/1.1".
// 17. SERVER_SOFTWARE: SERVER_SOFTWARE provides information about the web server software being used, including its name and version.





class CGI {
private:
    CGI();
    CGI(const CGI & cpy);
    CGI& operator=(const CGI & cpy);
    std::vector<char>       response;

public:
    CGI(const std::map<std::string, std::string> req, const t_conf_block& matching_directives){
		// set-up environment variables into a map
		function();
		// parse the urls
		const std::string				PATH_CGI;			// cgi path
		const std::string				PATH_INFO;			// relative add path
		const std::string				PATH_TRANSLATED;	// full add path	

        set_up_env();

    }
    ~CGI();

    launch(){

        // create an output file
        fork;

        if (pid == 0){      // child -> CGI
            //create a file
            // put the body in the file
            // if no body ? dup or not ?
            dup2(stdin, file_in);
            dup2(stdout, file_out);            
        }
        else if (pid == -1){

        }
        else {              // parent
            wait(); //check
            open fileout;
            read fileout;
        }
    }

    std::vector<char> getResponse(){return this->response;}
}

#endif