# include	"CGI.hpp"
# include	"utils.cpp"

# include	<sstream>
# include	<sys/socket.h>
# include	<arpa/inet.h>
# include	<unistd.h>			// fork, dup2, execve, write, close
# include	<sys/wait.h>		// wait
# include	<fcntl.h>			// open

//*		Public member functions
CGI::CGI(
	int											sock_fd,
	const std::string&							client_IP,
	const std::string&							server_IP,
	const std::map<std::string, std::string>	req,
	const t_conf_block&							matching_directives,
	std::string									cgi_ext
)
: sock_fd(sock_fd), client_IP(client_IP), server_IP(server_IP), url(req.at("url")),\
 cgi_extension(cgi_ext), path_info(), script_name(), query_str(), response(), req(req)
{
	init_paths();	// initialize path_info, script_name and query_str
	init_env();		// initialize environment variables
	print_arr(cgi_env, "CGI Environment");
}

CGI::~CGI()
{
	for (int i = 0 ; i < CGI_ENV_SIZE; i++)
		free(cgi_env[i]);
}

std::string CGI::get_env_value(const std::string & key){
	for (int i = 0; this->cgi_env[i]; ++i){
		std::string line = std::string(this->cgi_env[i]);
		size_t pos_key = line.find(key + "=");
		if (pos_key == 0) // should always be at the beginning of line
			return line.substr(key.size() + 1);
	}
	return std::string("");
}


		#include <fstream>
		#include <iostream>
void CGI::launch()
{
	pid_t pid = 0;

	// need to chose where to put the creation of files
		// file_in = open("cgi-bin/cgi_input.txt", O_RDONLY);

	pid = fork();
	if (pid == -1){
		throw SystemCallException("fork()");
	}
	else if (pid == 0){		// child -> CGI
	// do we need 0666 mode ?
	// create an input stream


		std::string input;
		if (req.find("body") != std::string::npos)
			input = req["body"];
		input += "\n" + get_env_value("QUERY_STRING") + "\n";
		// write to the input file the content of the request body/query
		
		std::fstream		instream(".cgi_input.txt", std::ios::in | std::ios::out | std::ios::trunc);
		if (false == instream.is_open())
			throw SystemCallException("is_open()"); // where, which function, which object
		instream << input;
		if (true == instream.fail())
			throw SystemCallException("<<()"); // where, which function, which object



	// create an output stream
		std::fstream		outstream(".cgi_output.txt", std::ios::in | std::ios::out | std::ios::trunc);
		if (false == outstream.is_open()) {
			throw SystemCallException("is_open()"); // where, which function, which object
		}
	// duping fd // if no body ? dup or not ? -> handled by CGI ?
		int fd_in = instream.rdbuf()->fd();
		int fd_out = outstream.rdbuf()->fd();

		if (dup2(fd_in, stdin) == -1)
			throw SystemCallException("dup2()");
		if (dup2(fd_out, stdout) == -1)
			throw SystemCallException("dup2()");
	// creates arguments for execve
			char ** cmd = 
	// executing cgi
		if (execve(cmd[0], cmd, this->cgi_env) == -1)
			throw SystemCallException("execve()");
	// close fd, free resources
		instream.close(); // not necessary, destructor when out of scope

		if (close(file_in) == -1)
			throw SystemCallException("close()");
		if (close(file_out) == -1)
			throw SystemCallException("close()");
		free_arr(av);
		free_arr(cmd);
		// put unlink 
	}
	else {              	// back to parent
		if (wait(0) == -1)
			throw SystemCallException("wait()");
		
		// use file_out to write the response
			// open fileout;
			// read fileout;
			// update response
		// free resources, close fd
		instream.close(); // not necessary, destructor when out of scope
	}
}

std::vector<char> CGI::getResponse()
{
	return this->response;
}

//*		Private member functions

// 23	REQUEST_URI		holds the original URI (Uniform Resource Identifier) sent by the client, including the path, query parameters, and fragments
// 24	SCRIPT_NAME		represents the path of the CGI script being executed, relative to the web server's document root
// 15	PATH_INFO		Optionally contains extra path information from the HTTP request that invoked the script, specifying a path to be interpreted by the CGI script. PATH_INFO identifies the resource or sub-resource to be returned by the CGI script, and it is derived from the portion of the URI path following the script name but preceding any query data.
// 17	QUERY_STRING	Contains the query parameters passed in the URL after the "?" character

// https://example.com/cgi_bin/index.php/path/to/resource?query=string",
// REQUEST_URI      would be "/cgi_bin/index.php/path/to/resource?query=string"
// SCRIPT_NAME      would be "cgi_bin/index.php"
// PATH_INFO        would be "path/to/resource"
// QUERY_STRING     would be "query=string"

void CGI::init_paths(){
	size_t		pos_ext, pos_query;

	pos_ext = this->url.find(this->cgi_extension); // cannot failed because checked before CGI obj created
	this->script_name = this->url.substr(0, pos_ext + this->cgi_extension.size());
	
	if (this->url.size() > (pos_ext + this->cgi_extension.size())){
		this->path_info = this->url.substr(pos_ext + this->cgi_extension.size());
		
		pos_query = this->path_info.find("?");
		if ((pos_query != std::string::npos) && (pos_query == this->path_info.size() - 1))
			this->query_str = this->path_info.substr(pos_query + 1);
			
		this->path_info = this->path_info.substr(0, pos_query);
	}

	path_remove_leading_slash(this->script_name);
	path_remove_leading_slash(this->path_info);
	path_remove_leading_slash(this->query_str);
}

void	CGI::init_env(){
	//*	we don't need to free it since the inet_toa function returns a statically allocated buffer
	char*				serverIP;
	struct sockaddr_in	sock_addr;
	socklen_t			len 		= sizeof(sock_addr);

	//! devo ricordarmi di toglierla da qui
	if (-1 == getsockname(sock_fd, (struct sockaddr*)&sock_addr, &len))
		throw (std::runtime_error("CGI() : getsockname() failed"));
	serverIP = inet_ntoa(sock_addr);

	cgi_env[0] = strdup(    (std::string("AUTH_TYPE=")                 +	std::string("")).c_str()  );
	cgi_env[1] = strdup(    (std::string("CONTENT_LENGTH=")            +	(req.find("Content-Length") != req.end()) ? req.at("Content-Length") : "").c_str()  );
	cgi_env[2] = strdup(    (std::string("CONTENT_TYPE=")              +	(req.find("Content-Type") != req.end()) ? req.at("Content-Type") : "").c_str()  );
	cgi_env[3] = strdup(    (std::string("GATEWAY_INTERFACE=")         +	std::string("CGI/1.1")).c_str()  );
	cgi_env[4] = strdup(    (std::string("HTTP_ACCEPT=")               +	(req.find("Accept") != req.end()) ? req.at("Accept") : "").c_str()	);
	cgi_env[6] = strdup(    (std::string("HTTP_ACCEPT_ENCODING=")      +	(req.find("Accept-Encoding") != req.end()) ? req.at("Accept-Encoding") : "").c_str()	);
	cgi_env[7] = strdup(    (std::string("HTTP_ACCEPT_LANGUAGE=")      +	(req.find("Accept-Language") != req.end()) ? req.at("Accept-Language") : "").c_str()	);
	cgi_env[8] = strdup(    (std::string("HTTP_COOKIE=")               +	(req.find("Cookie") != req.end()) ? req.at("Cookie") : "").c_str()	);
	cgi_env[9] = strdup(    (std::string("HTTP_FORWARDED=")            +	(req.find("Forwarded") != req.end()) ? req.at("Forwarded") : "").c_str()	);
	cgi_env[10] = strdup(   (std::string("HTTP_HOST=")                 +	(req.find("Host") != req.end()) ? req.at("Host") : "").c_str()	);
	cgi_env[11] = strdup(   (std::string("HTTP_PROXY_AUTHORIZATION=")  +	(req.find("Proxy-Authorization") != req.end()) ? req.at("Proxy-Authorization") : "").c_str()	);
	cgi_env[12] = strdup(   (std::string("HTTP_USER_AGENT=")           +	(req.find("User-Agent") != req.end()) ? req.at("User-Agent") : "").c_str()	);
	cgi_env[13] = strdup(   (std::string("NCHOME=")                    + 	std::string("")).c_str()   );

	cgi_env[22] = strdup(   (std::string("REQUEST_URI=")               + 	this->url).c_str()	);
	cgi_env[23] = strdup(   (std::string("SCRIPT_NAME=")               + 	this->script_name).c_str()   );
	cgi_env[16] = strdup(   (std::string("QUERY_STRING=")              + 	this->query_str).c_str()   );
	cgi_env[14] = strdup(   (std::string("PATH_INFO=")                 + 	this->path_info).c_str()   );
	cgi_env[15] = strdup(   (std::string("PATH_TRANSLATED=")           + 	take_location_root(matching_directives) + this->path_info : "").c_str()   );

//TODO
	cgi_env[17] = strdup(   (std::string("REMOTE_ADDR=")               + 	std::string(client_IP)).c_str()	);
	cgi_env[18] = strdup(   (std::string("REMOTE_HOST=")               + 	std::string("")).c_str()   );
	cgi_env[19] = strdup(   (std::string("REMOTE_IDENT=")              + 	std::string("")).c_str()   );
	cgi_env[20] = strdup(   (std::string("REMOTE_USER=")               + 	std::string("")).c_str()   );
	cgi_env[21] = strdup(   (std::string("REQUEST_METHOD=")            + 	req.at("method")).c_str()   );
	cgi_env[24] = strdup(   (std::string("SERVER_NAME=")               + 	std::string(server_IP)).c_str()				);
	cgi_env[25] = strdup(   (std::string("SERVER_PORT=")               + 	matching_directives.directives.at("listen")).c_str()	);
	cgi_env[26] = strdup(   (std::string("SERVER_PROTOCOL=")           + 	std::string("HTTP/1.1")).c_str()   );
	cgi_env[27] = strdup(   (std::string("SERVER_SOFTWARE=")           + 	std::string("WebServ_PiouPiou/1.0.0 (Ubuntu)")).c_str()   );
	cgi_env[28] = strdup(   (std::string("WEBTOP_USER=")               + 	std::string("")).c_str()   );
	cgi_env[CGI_ENV_SIZE] = NULL;
}

//* Debug
void CGI::print_arr(char ** arr, std::string& title){
	if (!arr)
		return;
	std::cout << "Printing " << title << std::endl;
	for(int i = 0; arr[i]; ++i)
		std::cout << i << " : " << arr[i] << std::endl;
}


//*		Unused default/copy constructor, copy operator
CGI() 
: sock_fd(), url(), cgi_extension(), path_info(), script_name(), query_str(), response() 
{}

CGI(const CGI & c)
: sock_fd(c.sock_fd), url(c.url), cgi_extension(c.cgi_extension), \
path_info(c.path_info), script_name(c.script_name), query_str(c.query_str), response(c.response) 
{}

CGI& operator=(const CGI & c){*this = c; return *this;}


