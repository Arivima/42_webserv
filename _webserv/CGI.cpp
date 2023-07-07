# include	"include/CGI.hpp"

# include	<sys/socket.h>
# include	<arpa/inet.h>
# include	<unistd.h>		// fork, dup2, execve, write, close
# include	<sys/wait.h>	// wait
# include	<fcntl.h>		// open
# include	<sys/stat.h>	// stat
# include	<sstream>
# include	<fstream>
# include	<iostream>
# include	<cstdio>

//*		Public member functions
CGI::CGI(
	int											sock_fd,
	const std::string&							client_IP,
	const std::string&							server_IP,
	const std::map<std::string, std::string>&	req,
	const t_conf_block&							matching_directives,
	const std::string&							location_root,
	const std::string &							cgi_extension,
	const std::string &							interpreter_path
)
	:	sock_fd(sock_fd),
		response(), req(req),
		matching_directives(matching_directives)
{
	std::cout << "CGI Constructor" << std::endl;

	init_env_paths(location_root, cgi_extension, interpreter_path);
	init_env(matching_directives, client_IP, server_IP);

	print_arr(this->cgi_env, std::string("CGI Environment"));
}

CGI::~CGI()
{
	for (int i = 0 ; i < CGI_ENV_SIZE; i++)
		free(cgi_env[i]);
}

std::vector<char> CGI::getResponse()
{
	return this->response;
}

std::string CGI::get_env_value(const std::string & key){
	for (int i = 0; this->cgi_env[i]; ++i){
		std::string line = std::string(this->cgi_env[i]);
		size_t pos_key = line.find(key + "=");
		if (pos_key == 0)
			return line.substr(key.size() + 1);
	}
	return std::string("");
}

void CGI::launch()
{COUT_DEBUG_INSERTION(YELLOW "CGI::launch()" RESET << std::endl);

	pid_t				 pid = 0;

	check_file_accessibility(
		X_OK,
		get_env_value("INTERPRETER_PATH"), "",
		matching_directives
	);
	check_file_accessibility(
		R_OK,
		get_env_value("SCRIPT_NAME"), get_env_value("ROOT"),
		matching_directives
	);
	pid = fork();
	if (pid == -1)
		throw_HttpError_debug("CGI::launch()", "fork()", 500, this->matching_directives, get_env_value("ROOT"));
	else if (pid == 0)  // child -> CGI
	{		
	// create the input string
		std::string input;
		input = get_env_value("QUERY_STRING");
		if (this->req.find("body") != this->req.end())
			input += this->req.at("body"); // redo
		
	// create an input file and write the content of body and query string
		std::ofstream	stream_cgi_infile(CGI_INFILE, std::ios::out | std::ios::trunc);
		if (false == stream_cgi_infile.is_open())
			throw_HttpError_debug("CGI::launch()", "is_open()", 500, this->matching_directives, get_env_value("ROOT"));
		
		stream_cgi_infile << input;
		
		if (stream_cgi_infile.fail())
			throw_HttpError_debug("CGI::launch()", "stream <<", 500, this->matching_directives, get_env_value("ROOT"));
		stream_cgi_infile.close();

	// open input and output files
		int fd_in = open(CGI_INFILE, O_RDONLY, 0666);
		if (fd_in == -1)
			throw_HttpError_debug("CGI::launch()", "open CGI_INFILE", 500, this->matching_directives, get_env_value("ROOT"));
		int fd_out = open(CGI_OUTFILE, O_RDWR | O_CREAT | O_TRUNC, 0666);
		if (fd_out == -1)
			throw_HttpError_debug("CGI::launch()", "open CGI_OUTFILE", 500, this->matching_directives, get_env_value("ROOT"));

	// duping fd // if no body ? dup or not ? -> handled by CGI ?
		if (dup2(fd_in, STDIN_FILENO) == -1)
			throw_HttpError_debug("CGI::launch()", "dup2(fd_in, STDIN_FILENO)", 500, this->matching_directives, get_env_value("ROOT"));
		if (dup2(fd_out, STDOUT_FILENO) == -1)
			throw_HttpError_debug("CGI::launch()", "dup2(fd_out, STDOUT_FILENO)", 500, this->matching_directives, get_env_value("ROOT"));
	// creates arguments for execve
		char* const cmd[3] = {
			strdup(get_env_value("INTERPRETER_PATH").c_str()),
			strdup((get_env_value("ROOT") + get_env_value("SCRIPT_NAME")).c_str()),
			nullptr
		};
	// executing cgi
		if (execve(cmd[0], cmd, this->cgi_env) == -1)
			throw_HttpError_debug("CGI::launch()", "execve()", 500, this->matching_directives, get_env_value("ROOT"));
	}
	else 	// back to parent
	{              
		if (wait(0) == -1)
			throw_HttpError_debug("CGI::launch()", "wait()", 500, this->matching_directives, get_env_value("ROOT"));
		// Update the response
		std::ifstream	stream_cgi_outfile(CGI_OUTFILE, std::ios::in);
		if (false == stream_cgi_outfile.is_open())
			throw_HttpError_debug("CGI::launch()", "is_open()", 500, this->matching_directives, get_env_value("ROOT"));
        
		response.assign(std::istreambuf_iterator<char>(stream_cgi_outfile), std::istreambuf_iterator<char>());
        
	// delete files
		if (unlink(CGI_OUTFILE) == -1) 
			throw_HttpError_debug("CGI::launch()", "unlink(CGI_OUTFILE)", 500, this->matching_directives, get_env_value("ROOT"));
		if (unlink(CGI_INFILE) == -1) 
			throw_HttpError_debug("CGI::launch()", "unlink(CGI_INFILE)", 500, this->matching_directives, get_env_value("ROOT"));
	}
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

void CGI::init_env_paths(
	const std::string & root, 
	const std::string & cgi_extension, 
	const std::string & interpreter_path
	)
{
	std::string	url(req.at("url"));
	std::string	path_info; 
	std::string	script_name; 
	std::string	query_str; 
	size_t		pos_ext;
	size_t		pos_query;


	pos_ext = url.find(cgi_extension);
	script_name = url.substr(0, pos_ext + cgi_extension.size());
	
	if (url.size() > (pos_ext + cgi_extension.size())){
		path_info = url.substr(pos_ext + cgi_extension.size());
		path_info = path_info.substr(0, path_info.find("?"));//*corretto

		pos_query = url.find("?");
		if (std::string::npos != pos_query && url.length() - 1 != pos_query)
			query_str = url.substr(pos_query + 1);
		else
			query_str = "";

		// if ((pos_query != std::string::npos) && (pos_query == path_info.size() - 1))
		// 	query_str = path_info.substr(pos_query + 1);
			
		//!wrong path_info = path_info.substr(0, pos_query);
	}
	path_remove_leading_slash(script_name);
	path_remove_leading_slash(path_info);
	path_remove_leading_slash(query_str);
	
	cgi_env[0] = strdup((std::string("ROOT=")           			+ 	root ).c_str())   ;
	cgi_env[1] = strdup((std::string("INTERPRETER_PATH=")			+ 	interpreter_path ).c_str())   ;
	cgi_env[2] = strdup((std::string("REQUEST_URI=")				+ 	url).c_str())	;
	cgi_env[3] = strdup((std::string("SCRIPT_NAME=")				+ 	script_name).c_str())   ;
	cgi_env[4] = strdup((std::string("QUERY_STRING=")				+ 	query_str).c_str())   ;
	cgi_env[5] = strdup((std::string("PATH_INFO=")					+ 	path_info).c_str())   ;
	cgi_env[6] = strdup((std::string("PATH_TRANSLATED=")			+ 	root + path_info).c_str())   ;

}

void	CGI::init_env(const t_conf_block& matching_directives, const std::string& client_IP, const std::string& server_IP){
	// const std::string&	client_IP;	//*	ip of remote client
	// const std::string&	server_IP;	//*	the interface, among all the assigned_server utilized interfaces, where the connection got accepted.

	// Path environment variables declared in init_env_paths();
	cgi_env[7] = strdup((std::string("AUTH_TYPE=")               +	std::string("")).c_str());
	cgi_env[8] = strdup((std::string("CONTENT_LENGTH=")          +	((req.find("Content-Length") != req.end()) ? req.at("Content-Length") : "")).c_str());
	cgi_env[9] = strdup((std::string("CONTENT_TYPE=")            +	((req.find("Content-Type") != req.end()) ? req.at("Content-Type") : "")).c_str());
	cgi_env[10] = strdup((std::string("GATEWAY_INTERFACE=")       +	std::string("CGI/1.1")).c_str());
	cgi_env[11] = strdup((std::string("HTTP_ACCEPT=")             +	((req.find("Accept") != req.end()) ? req.at("Accept") : "")).c_str());
	cgi_env[12] = strdup((std::string("HTTP_ACCEPT_ENCODING=")    +	((req.find("Accept-Encoding") != req.end()) ? req.at("Accept-Encoding") : "")).c_str());
	cgi_env[13] = strdup((std::string("HTTP_ACCEPT_LANGUAGE=")    +	((req.find("Accept-Language") != req.end()) ? req.at("Accept-Language") : "")).c_str());
	cgi_env[14] = strdup((std::string("HTTP_COOKIE=")             +	((req.find("Cookie") != req.end()) ? req.at("Cookie") : "")).c_str());
	cgi_env[15] = strdup((std::string("HTTP_FORWARDED=")          +	((req.find("Forwarded") != req.end()) ? req.at("Forwarded") : "")).c_str());
	cgi_env[16] = strdup((std::string("HTTP_HOST=")                 			+	((req.find("Host") != req.end()) ? req.at("Host") : "")).c_str());
	cgi_env[17] = strdup((std::string("HTTP_PROXY_AUTHORIZATION=")  			+	((req.find("Proxy-Authorization") != req.end()) ? req.at("Proxy-Authorization") : "")).c_str());
	cgi_env[18] = strdup((std::string("HTTP_USER_AGENT=")           			+	((req.find("User-Agent") != req.end()) ? req.at("User-Agent") : "")).c_str());
	cgi_env[19] = strdup((std::string("NCHOME=")                    			+ 	std::string("")).c_str());
	cgi_env[20] = strdup((std::string("REMOTE_ADDR=")               			+ 	std::string(client_IP)).c_str());
	cgi_env[21] = strdup((std::string("REMOTE_HOST=")               			+ 	std::string("")).c_str());
	cgi_env[22] = strdup((std::string("REMOTE_IDENT=")              			+ 	std::string("")).c_str());
	cgi_env[23] = strdup((std::string("REMOTE_USER=")               			+ 	std::string("")).c_str());
	cgi_env[24] = strdup((std::string("REQUEST_METHOD=")            			+ 	req.at("method")).c_str());
	cgi_env[25] = strdup((std::string("SERVER_NAME=")               			+ 	std::string(server_IP)).c_str());
	cgi_env[26] = strdup((std::string("SERVER_PORT=")               			+ 	matching_directives.directives.at("listen")).c_str());
	cgi_env[27] = strdup((std::string("SERVER_PROTOCOL=")           			+ 	std::string("HTTP/1.1")).c_str());
	cgi_env[28] = strdup((std::string("SERVER_SOFTWARE=")           			+ 	std::string("WebServ_PiouPiou/1.0.0 (Ubuntu)")).c_str());
	cgi_env[29] = strdup((std::string("WEBTOP_USER=")               			+ 	std::string("")).c_str());
	cgi_env[CGI_ENV_SIZE] = NULL;
}

//* Debug
void CGI::print_arr(char ** arr, const std::string& title)
{COUT_DEBUG_INSERTION(YELLOW "CGI::print_arr()" RESET << std::endl);

	if (!arr)
		return;
	std::cout << "Printing " << title << std::endl;
	for(int i = 0; arr[i]; ++i)
		std::cout << i << " : " << arr[i] << std::endl;

	COUT_DEBUG_INSERTION(YELLOW "END_____CGI::print_arr()" RESET << std::endl)
}
