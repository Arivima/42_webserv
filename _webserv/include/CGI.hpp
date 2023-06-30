#ifndef		CGI_HPP
# define	CGI_HPP

# include	"Webserv.hpp"
# include	<map>
# include	<vector>
# include	<string>

# define	CGI_ENV_SIZE	29

//? N.B : see dictionnary of all ENV variabbles at end of this file

class CGI {
	//*		Private member attributes
	private:
		const int			sock_fd;
		char*				cgi_env[CGI_ENV_SIZE + 1];
		std::vector<char>	response;
		const std::map<std::string, std::string> &	req;

	//*		Public member functions
	public:
		CGI(
			int											sock_fd,
			const std::string&							client_IP,
			const std::string&							server_IP,
			const std::map<std::string, std::string>	req,
			const t_conf_block&							matching_directives,
			const std::string &							interpreter_path,
			const std::string &							cgi_ext
		);
		~CGI();
		void				launch();
		std::vector<char>	getResponse();
		std::string			get_env_value(const std::string & key);

	//*		Private member functions
	private:
		void				init_env_paths(std::string root, std::string cgi_extension);
		void				init_env(const t_conf_block& matching_directives, const std::string& client_IP, const std::string& server_IP);

		//* Unused default/copy constructor, copy operator
		CGI();
		CGI(const CGI & c);
		CGI&				operator=(const CGI & c);

		//* Debug
		void				print_arr(char ** arr, const std::string & title);
}
#endif

// DICTIONNARY OF ALL CGI PROTOCOL ENVIRONMENT VARIABLES
// env_type : char** env
// 1	AUTH_TYPE               This variable contains the authentication type used for the current request, typically provided by the web server
// 2	CONTENT_LENGTH	        This variable holds the size, in bytes, of the request body (the content being sent in the request)
// 3	CONTENT_TYPE	        The MIME type of the body of the request, or null if the type is not known. For HTTP servlets, the value returned is the same as the value of the CGI variable CONTENT_TYPE.
// 4	GATEWAY_INTERFACE	    The revision of the CGI specification being used by the server to communicate with the script. It is "CGI/1.1".
// 5	HTTP_ACCEPT	            Variables with names beginning with "HTTP_" contain values from the request header, if the scheme used is HTTP. HTTP_ACCEPT specifies the content types your browser supports. For example, text/xml.
// 6	HTTP_ACCEPT_CHARSET	    Character preference information. Used to indicate the client's prefered character set if any. For example, utf-8;q=0.5.
// 7	HTTP_ACCEPT_ENCODING	Defines the type of encoding that may be carried out on content returned to the client. For example, compress;q=0.5.
// 8	HTTP_ACCEPT_LANGUAGE	Used to define which languages you would prefer to receive content in. For example, en;q=0.5. If nothing is returned, no language preference is indicated.
// 9	HTTP_COOKIE	            Cookie String.
// 10	HTTP_FORWARDED	        If the request was forwarded, shows the address and port through of the proxy server.
// 11	HTTP_HOST	            Specifies the Internet host and port number of the resource being requested. Required for all HTTP/1.1 requests.
// 12	HTTP_PROXY_AUTHORIZATION	Used by a client to identify itself (or its user) to a proxy which requires authentication.
// 13	HTTP_USER_AGENT	        The type and version of the browser the client is using to send the request. For example, Mozilla/1.5.
// 14	NCHOME	                The NCHOME environment variable.
// 15	PATH_INFO	            Optionally contains extra path information from the HTTP request that invoked the script, specifying a path to be interpreted by the CGI script. PATH_INFO identifies the resource or sub-resource to be returned by the CGI script, and it is derived from the portion of the URI path following the script name but preceding any query data.
// 16	PATH_TRANSLATED	        Represents the translated file path corresponding to the PATH_INFO
// 17	QUERY_STRING	        Contains the query parameters passed in the URL after the "?" character
// 18	REMOTE_ADDR	            This variable holds the IP address of the client making the request
// 19	REMOTE_HOST	            The hostname making the request. If the server does not have this information, it should set REMOTE_ADDR and leave this unset.
// 20	REMOTE_IDENT	        If the HTTP server supports RFC 931 identification, then this variable will be set to the remote user name retrieved from the server. Usage of this variable should be limited to logging only.
// 21	REMOTE_USER	            Returns the login of the user making this request if the user has been authenticated, or null if the user has not been authenticated.
// 22	REQUEST_METHOD	        Returns the name of the HTTP method with which this request was made. For example, GET, POST, or PUT.
// 23	REQUEST_URI	            Holds the original URI (Uniform Resource Identifier) sent by the client, including the path, query parameters, and fragments
// 24	SCRIPT_NAME             Represents the path of the CGI script being executed, relative to the web server's document root
// 25	SERVER_NAME             Contains the domain name or IP address of the server handling the request
// 26	SERVER_PORT             Holds the port number on which the server is listening for requests, typically 80 for HTTP and 443 for HTTPS
// 27	SERVER_PROTOCOL	        This variable specifies the name and version of the protocol used by the server to handle the request, such as "HTTP/1
// 28	SERVER_SOFTWARE	        Provides information about the web server software being used, including its name and version
// 29	WEBTOP_USER	            The user name of the user who is logged in.
