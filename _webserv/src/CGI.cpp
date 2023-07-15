/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinel <mmarinel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/13 17:23:17 by mmarinel          #+#    #+#             */
/*   Updated: 2023/07/15 14:17:01 by mmarinel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include	"CGI.hpp"
# include	"Utils.hpp"

# include	<sys/socket.h>
# include	<arpa/inet.h>
# include	<unistd.h>		// fork, dup2, execve, write, close
# include	<fcntl.h>		// open
# include	<sys/stat.h>	// stat
# include	<sstream>
# include	<fstream>
# include	<iostream>
# include	<cstdio>
# include	<cstring>

//*		Public member functions
CGI::CGI(
	int											sock_fd,
	const std::string&							client_IP,
	const std::string&							server_IP,
	Request*									request,
	bool										chunked,
	const t_conf_block&							matching_directives,
	const std::string&							location_root,
	const std::string &							cgi_extension,
	const std::string &							interpreter_path
)
	:	sock_fd(sock_fd),
		response(), request(request), req(request->getRequest()),
		matching_directives(matching_directives)
{
	init_env_paths(location_root, cgi_extension, interpreter_path);
	init_env(matching_directives, client_IP, server_IP);

	this->chunked = chunked;
	max_body_size = std::atol(matching_directives.directives.at("body_size").c_str());

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

	if (-1 == pipe(sendPayload_pipe))
		throw_HttpError_debug("CGI::launch()", "pipe()", 500, this->matching_directives, get_env_value("ROOT"));
	
	pid = fork();
	if (pid == -1)
		throw_HttpError_debug("CGI::launch()", "fork()", 500, this->matching_directives, get_env_value("ROOT"));
	else if (pid == 0)  // child -> CGI
	{
		//*	redirecting script's STDIN to PIPE
		if (-1 == dup2(sendPayload_pipe[0], STDIN_FILENO))
			throw_HttpError_debug("CGI::launch()", "dup2()", 500, this->matching_directives, get_env_value("ROOT"));
		close(sendPayload_pipe[1]);

		//*	creating outfile fd and dupping to STDOUT
		fd_out = open(CGI_OUTFILE, O_RDWR | O_CREAT | O_TRUNC, 0666);
		if (fd_out == -1)
			throw_HttpError_debug("CGI::launch()", "open CGI_OUTFILE", 500, this->matching_directives, get_env_value("ROOT"));
		if (dup2(fd_out, STDOUT_FILENO) == -1)
			throw_HttpError_debug("CGI::launch()", "dup2(fd_out, STDOUT_FILENO)", 500, this->matching_directives, get_env_value("ROOT"));
		close(fd_out);

		//* creates arguments for execve
		char* const cmd[3] = {
			strdup(get_env_value("INTERPRETER_PATH").c_str()),
			strdup((get_env_value("ROOT") + get_env_value("SCRIPT_NAME")).c_str()),
			nullptr
		};
		//* executing cgi
		if (execve(cmd[0], cmd, this->cgi_env) == -1) {
			//! take care of leaks
			exit(EXIT_FAILURE);
			throw_HttpError_debug("CGI::launch()", "execve()", 500, this->matching_directives, get_env_value("ROOT"));
		}
	}
	else 	// back to parent
	{
		close(sendPayload_pipe[0]);
		if (false == chunked)
		{
			COUT_DEBUG_INSERTION("CGI NOT CHUNKED" << std::endl);
			//*	printing body into the pipe
			backupStdout = dup(STDOUT_FILENO);
			dup2(sendPayload_pipe[1], STDOUT_FILENO);

			std::cout.write(request->getPayload().data(), request->getPayload().size());
			std::cout.flush();
				//*	putting back STDOUT where it belongs
			dup2(backupStdout, STDOUT_FILENO);
			close(backupStdout);
			close(sendPayload_pipe[1]);

			//*	waiting for cgi to terminate
			if (waitpid(pid, NULL, 0) == -1)
				throw_HttpError_debug("CGI::launch()", "waitpid()", 500, this->matching_directives, get_env_value("ROOT"));

			//* Update the response
			std::ifstream	stream_cgi_outfile(CGI_OUTFILE, std::ios::in);
			if (false == stream_cgi_outfile.is_open())
				throw_HttpError_debug("CGI::launch()", "is_open()", 500, this->matching_directives, get_env_value("ROOT"));
			response.assign(std::istreambuf_iterator<char>(stream_cgi_outfile), std::istreambuf_iterator<char>());

			//* delete files
			stream_cgi_outfile.close();
			if (unlink(CGI_OUTFILE) == -1) 
				throw_HttpError_debug("CGI::launch()", "unlink(CGI_OUTFILE)", 500, this->matching_directives, get_env_value("ROOT"));
		}
	}
}


void	CGI::CGINextChunk( void )
{
	std::vector<char>	incomingData;

		try
		{
			incomingData = request->getIncomingData();
		}
		catch (const ChunkNotComplete& e) {
			return ;
		}

		if (incomingData.empty())
		{
			COUT_DEBUG_INSERTION("EOF CHUNK FOUND" << std::endl);
			//*	sending EOF to the CGI and waiting the script to be done
			close(sendPayload_pipe[1]);
			if ( -1 == waitpid(pid, NULL, 0) )
				throw_HttpError_debug(
					"CGI::CGINextChunk()", "waitpid",
					500,
					matching_directives, get_env_value("ROOT")
				);
			
			//* Update the response
			std::ifstream	stream_cgi_outfile(CGI_OUTFILE, std::ios::in);
			if (false == stream_cgi_outfile.is_open())
				throw_HttpError_debug("CGI::launch()", "is_open()", 500, this->matching_directives, get_env_value("ROOT"));
			response.assign(std::istreambuf_iterator<char>(stream_cgi_outfile), std::istreambuf_iterator<char>());

			//* delete files
			stream_cgi_outfile.close();
			if (unlink(CGI_OUTFILE) == -1) 
				throw_HttpError_debug("CGI::launch()", "unlink(CGI_OUTFILE)", 500, this->matching_directives, get_env_value("ROOT"));
			
			//*	signaling end of work
			chunked = false;
			throw TaskFulfilled();
		}
		else
		{
			backupStdout = dup(fileno(stdout));
			dup2(sendPayload_pipe[1], fileno(stdout));
			std::cout.write(incomingData.data(), incomingData.size());
			std::cout.flush();
			max_body_size -= incomingData.size();
			if (max_body_size < 0)
			{
				chunked = false;
				dup2(backupStdout, fileno(stdout));
				close(backupStdout);
				//*	sending EOF to the CGI and waiting the script to be done
				close(sendPayload_pipe[1]);
				waitpid(pid, NULL, 0);
				throw HttpError(413, matching_directives, get_env_value("ROOT"));
			}
			dup2(backupStdout, fileno(stdout));
			close(backupStdout);
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
	cgi_env[30] = strdup((std::string("MAX_BODY_SIZE=")							+	matching_directives.directives.at("body_size")).c_str());
	cgi_env[CGI_ENV_SIZE] = NULL;
}

//* Debug
void CGI::print_arr(char ** arr, const std::string& title)
{
	if (DEBUG){
		std::cout << YELLOW "CGI::print_arr()" RESET << std::endl;

		if (!arr)
			return;
		std::cout << "Printing " << title << std::endl;
		for(int i = 0; arr[i]; ++i)
			std::cout << i << " : " << arr[i] << std::endl;

		std::cout << YELLOW "END_____CGI::print_arr()" RESET << std::endl;		
	}

}
