/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/08 17:25:54 by earendil          #+#    #+#             */
/*   Updated: 2023/06/08 17:49:19 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "include/Response.hpp"
#include <sys/socket.h>	//send
#include <fstream>		//open requested files
#include <sstream>		//stringstream
#include <cstring>		//memeset
#include <list>			//location matches sorting
#include <unistd.h>		// access, unlink
#include <cerrno>		// errno
#include <stdio.h>		// remove
#include <iostream>
#include <string>
#include <dirent.h>

//*		non-member helper functions
//*		/////////////////////////////////////////////////


//*		Main Constructor and Destructor
Response::Response(
	const std::map<std::string, std::string>& req,
	const t_server& assigned_server,
	const int sock_fd,
	const std::string& client_IP,
	const std::string& server_IP,
	const t_epoll_data& edata
)
		:
		matching_directives(takeMatchingDirectives(assigned_server.conf_server_block, req)),
		req(req),
		assigned_server(assigned_server),
		sock_fd(sock_fd),
		client_IP(client_IP),
		server_IP(server_IP),
		edata(edata)
{
}

Response::~Response() {
	response.clear();
}


//*		Main Functionalities

void	Response::send_line( void )
{
	const struct epoll_event*	eevent = edata.getEpollEvent(this->sock_fd);
	int							bytes_sent;

	COUT_DEBUG_INSERTION("send_line()\n")
	if (response.empty()) {
		COUT_DEBUG_INSERTION(
			"Response::send_line() ---TaskFulfilled"
			<< std::endl
		);
		throw TaskFulfilled();
	}
	else {
		if (NULL != eevent && eevent->events & EPOLLOUT)
		{
			std::string	debug(this->response.begin(), this->response.end());
			COUT_DEBUG_INSERTION(
				"sending chars : "
				<< "|" << debug << "|"
				<< std::endl
			);

			bytes_sent = send(
				this->sock_fd, response.data(), response.size(), 0
			);
			if (bytes_sent < 0)
				/*Server Err*/	throw HttpError(500, matching_directives, take_location_root(matching_directives, false));
			else
			if (0 == bytes_sent)
				throw TaskFulfilled();
			else
				response.erase(
					response.begin(), response.begin() + bytes_sent
				);
		}
	}
}

void	Response::generateResponse( void )
{
	std::string		cgi_extension;
	std::string		cgi_interpreter_path;
	std::string		url_no_query_str = uri_remove_queryString(req.at("url"));
	try
	{
		if (false == isMethodAllowed())
			throw (HttpError(405, matching_directives, take_location_root(matching_directives, false)));
		
		cgi_extension = take_cgi_extension(
			req.at("url"), matching_directives.directives
		);
		if (false == cgi_extension.empty())
		{
			std::cout << GREEN << "Response::generateResponse there is a CGI extension" << RESET << std::endl;
			
			cgi_interpreter_path = take_cgi_interpreter_path(
										cgi_extension,
										matching_directives.directives.at("cgi_enable")//* safe to call in this branch
									);
			COUT_DEBUG_INSERTION(
				"cgi interpreter full path : |"
				<< cgi_interpreter_path
				<< "|"
				<< std::endl
			);
			this->cgi = new CGI(sock_fd, client_IP, server_IP, req, matching_directives, cgi_interpreter_path);
			this->cgi->launch();
			this->response = this->cgi->getResponse();

			std::string	debug(this->response.begin(), this->response.end());
			COUT_DEBUG_INSERTION(
				"CGI response : " << std::endl
				<< "|" << debug << "|" << std::endl;
			);
			return ;
		}
		if ("GET" == this->req.at("method"))
			return (generateGETResponse(url_no_query_str));
		// if ("POST" == this->req.at("method"))
		// 	return (generatePOSTResponse(url_no_query_str));
		// if ("PUT" == this->req.at("method"))
		// 	return (generatePUTResponse());
		if ("DELETE" == this->req.at("method"))
			return (generateDELETEResponse(url_no_query_str));
		throw (HttpError(501, this->matching_directives, take_location_root(matching_directives, false)));
	}
	catch (const HttpError& e) {
		this->response = e.getErrorPage();
	}
}

//*		Main Helper Functions

/**
 * @brief This function returns a handle to a generic block holding directives.
 * Given a server block, it returns the inner location block that matches
 * the current request, or the server block itself if no location block matches.
 * @param conf_server_block 
 * @return const t_conf_block& - the block of directives that applies to the current http request
 */
const t_conf_block&	Response::takeMatchingDirectives(
	const t_conf_block& conf_server_block,
	const std::map<std::string, std::string>& req
	)
{
	const t_conf_block&							virtual_server  = takeMatchingServer(
		conf_server_block.sub_blocks, req
	);
	std::vector<t_conf_block>::const_iterator	location;
	std::list<t_location_match>					matches;
	size_t										match_score;
	
	for (
		location = virtual_server. sub_blocks.begin();
		location != virtual_server.sub_blocks.end();
		location ++
	)
	{
		if ( (match_score = locationMatch(*location, req.at("url"))) )
			matches.push_back(t_location_match(*location, match_score));
	}

	if (matches.empty()) {
		COUT_DEBUG_INSERTION(
			RED "no location for requested url" RESET
			 << std::endl
		)
		return (virtual_server);
	}
	else {
		matches.sort();
		COUT_DEBUG_INSERTION(
			BOLDGREEN "taking location : " << matches.back().location.directives.at("root") << RESET
			<< std::endl
		)
		return (matches.back().location);
	}
}

/**
 * @brief This function returns a handle to a generic block holding directives.
 * This function returns server block that matches
 * the current request.
 * 
 * @param virtual_servers 
 * @return const t_virtual_server_block& 
 */
const t_conf_block&	Response::takeMatchingServer(
		const std::vector<t_conf_block>&	virtual_servers,
		const std::map<std::string, std::string>& req
		)
{
	std::vector<t_conf_block>::const_iterator virtual_server;

	for (
		virtual_server = virtual_servers.begin();
		virtual_server != virtual_servers.end();
		virtual_server ++
	)
		if (
			(*virtual_server).directives.end() != (*virtual_server).directives.find("server_name") &&
			req.end() != req.find("Host") &&
			(*virtual_server).directives.at("server_name") == req.at("Host"))
			break ;
		
	//*		if no Host is matched, choose the default server
	return (
		virtual_server == virtual_servers.end() \
			? virtual_servers[0] \
			: (*virtual_server)
	);
}

/**
 * @brief this function returns matching information of a location block for a request url.
 * 
 * @param location 
 * @param req_url 
 * @return size_t match score. 0 for no match, npos (maximum) for exact match
 * or length of the block's location for prefix match.
 */
size_t	Response::locationMatch(
	const t_conf_block& location, const std::string& req_url
	)
{
	size_t		match_score;
	size_t		query_arguments_start;
	std::string	location_path;
	std::string	reqUrl = req_url;

	//*		removing optional query string part
	query_arguments_start = reqUrl.find("?");
	if (std::string::npos != query_arguments_start)
		reqUrl = reqUrl.substr(0, query_arguments_start);

	if (0 == location.directives.at("location").find("="))
	{
		//*		exact match
		location_path = location.directives.at("location").substr(1);
		strip_trailing_and_leading_spaces(location_path);
		COUT_DEBUG_INSERTION(
			"trying exact match on location |" << location_path << "|"
			<< " with uri : |" << reqUrl << "|"
			<< std::endl
		)
		if (0 == reqUrl.compare(location_path)
		)
			match_score = (std::string::npos);
		else
			match_score = (0);
	}
	else
	{
		location_path = location.directives.at("location");
		COUT_DEBUG_INSERTION(
			"trying prefix match on location |" << location_path << "|"
			<< " with uri : |" << reqUrl << "|"
			<< std::endl
		)
		//*		prefix match
		if (0 == reqUrl.find(location_path))
			match_score = (location_path.length());
		else
			match_score = (0);
	}

	COUT_DEBUG_INSERTION(
		" match score : " << match_score << std::endl
	)
	return (match_score);
}

//_______________________ METHOD : GET _______________________
// refactor to clean + test and make sure all cases are covered
void	Response::generateGETResponse(  const std::string uri_path  )
{COUT_DEBUG_INSERTION(YELLOW "Response::generateGETResponse()" RESET << std::endl);

	const std::string				root = take_location_root(matching_directives, false);
	const std::string				reqPath(http_req_complete_url_path(uri_path, root));
	std::string						headers;
	std::vector<char>				page;
	std::string						filePath = "";

	std::cout << "Path of the request : " << (reqPath.empty()? "EMPTY" : reqPath ) << std::endl;
	if (reqPath.empty()) {
		if (
			(this->matching_directives.directives.find("autoindex") != this->matching_directives.directives.end())
			&& (this->matching_directives.directives.at("autoindex") == "on" ))
		{
			std::string	path = req.at("url");
			path_remove_leading_slash(path);
			COUT_DEBUG_INSERTION("showing dir listing for " << root + path << std::endl)
			std::string dir_listing_page = createHtmlPage(
				"Directory Listing for /" + path,
				getDirectoryContentList(root + path) //wip
			);
			page.insert(
				page.begin(),
				dir_listing_page.begin(),
				dir_listing_page.end()
			);
			headers = getHeaders(200, "OK", filePath, page.size());
			// throw (std::runtime_error("not yet implemented"));
			// finire gestire ls
		}
		else {
			std::cout << "autoindex not set" << std::endl;
			/*Not found*/	throw HttpError(404, this->matching_directives, root);
		}
	}
	else {
		COUT_DEBUG_INSERTION("serving page : " << root + reqPath << std::endl)
										filePath = root + reqPath;
		std::ifstream					docstream(filePath.c_str(), std::ios::binary);

		if (false == docstream.is_open()) {
			COUT_DEBUG_INSERTION("throwing page not found\n")
			/*Not found*/	throw HttpError(404, matching_directives, take_location_root(matching_directives, false));
		}
		try {
			page.insert(
				page.begin(),
				std::istreambuf_iterator<char>(docstream),
				std::istreambuf_iterator<char>());
		}
		catch (const std::exception& e) {
			COUT_DEBUG_INSERTION("throwing Internal Server Error\n")
			/*Server Err*/	throw HttpError(500, matching_directives, take_location_root(matching_directives, false));
		}
		headers = getHeaders(200, "OK", filePath, page.size());
	}
	response.insert(response.begin(), headers.begin(), headers.end());
	response.insert(response.end(), page.begin(), page.end());
}

//*	considerare se usare lstat per leggere nei primi byte del file il tipo
//*	(l'estensione potrebbe essere stata cancellata)
std::string		Response::getHeaders(
	int status, std::string description, std::string& filepath,
	size_t body_size, std::string location_header
	)
{
	std::stringstream	headersStream;
	size_t				dot_pos;
	std::string			fileType = "";
	std::string			fileType_prefix;

	if ("" == filepath)//* not specified : means we are creating an html page on the fly
		fileType = ".html";
	else {
		dot_pos = filepath.rfind('.');
		if (std::string::npos != dot_pos)//* the html page is an actual file, we take the position of the file extension
			fileType = filepath.substr(dot_pos);
		//*else //the html page is an actual file, but does not name a file extension
	}

	if (".html" == fileType || ".css" == fileType)
		fileType_prefix = "text/";
	else if (".png" == fileType || ".jpg" == fileType || ".jpeg" == fileType || ".gif" == fileType)
		fileType_prefix = "image/";
	else if (".json" == fileType || ".js" == fileType)
		fileType_prefix = "application/";
	else {
		fileType_prefix = "text/";
		fileType = ".plain";
	}
	//*		removing the dot '.'
	fileType = fileType.substr(1);
	COUT_DEBUG_INSERTION("fileType : |" << fileType << "|"<< std::endl);
	headersStream
		<< "HTTP/1.1 " << status << " " << description << "\r\n"
		<< "Content-Type: " << fileType_prefix << fileType << "\r\n"
		<< "Content-Length : " << body_size << "\r\n"
		<< (location_header.empty() ? "" : location_header + "\r\n")
		<< "\r\n";
	
	return (headersStream.str());
}

std::string		Response::getIndexPage( const std::string& root, std::string path )
{COUT_DEBUG_INSERTION(YELLOW "Response::getIndexPage()" RESET << std::endl);

	std::string									indexes;
	std::stringstream							indexesStream;
	std::string									cur_index;
	const std::map<std::string, std::string>&	directives
		= this->matching_directives.directives;
	
	if ('/' != path[path.length() - 1])
		path += "/";
	if (directives.end() != directives.find("index"))
	{
		indexes = directives.at("index");
		indexesStream.str(indexes);

		while (indexesStream.good()) {
			getline(indexesStream, cur_index, ' ');
			path_remove_leading_slash(cur_index);
			COUT_DEBUG_INSERTION("trying index file : "\
				 << root + path + cur_index \
				 << std::endl)
			std::ifstream	file(root + path + cur_index);
			if (file.is_open())
			{
				file.close();
				return (path + cur_index);
			}
		}
		return ("");
	}
	else
		return ("");
}

//*		Secondary Helper Functions

bool	Response::isMethodAllowed(void)
{
	std::stringstream	methodDirectiveStream;
	std::string			cur_method;

	if ("GET" == req.at("method"))
		return (true);
	if (matching_directives.directives.end() == matching_directives.directives.find("method"))
		return (false);
	
	methodDirectiveStream.str(matching_directives.directives.at("method"));
	while (std::getline(methodDirectiveStream, cur_method, ' '))
	{
		if (req.at("method") == cur_method)
			return (true);
	}
	return (false);
}

/**
 * @brief this function completes the path-part of a request URL.
 * That means it´s going to fecth a index file if the requested resource is a directory.
 * 
 * @param uri - the path-part of a request url without query string arguments
 * @param root - the root directory for the requested location (i.e.: var/www/html)
 * @return std::string : a relative path (any leading slash will be cleaned) pointing to a static resource
 * or empty in case of error (e.g.: no index file could be found)
 */
std::string		Response::http_req_complete_url_path(
	const std::string& uri, const std::string& root
	)
{COUT_DEBUG_INSERTION(YELLOW "Response::http_req_complete_url_path()" RESET << std::endl);

	std::string		path = uri;
	size_t			path_start;
	
	//*	protection against telnet bad requests	(i.e.: when we write the http request by hand)
	if (
		std::string::npos == (path_start = uri.find("/"))
	)
	{
		/*Bad req*/	throw HttpError(400, matching_directives, root);
	}
	path = uri.substr(path_start);
	//****************************************************
	
	//*	check if requested resource is a directory
	if ( true == isDirectory( root, path, this->matching_directives) ) {
		path = getIndexPage(root, path);
	}
	// else path refers to a regular file and is already correct

	path_remove_leading_slash(path);
	return (path);//*	may be empty (a.k.a. "")
}












/**
 * @brief this function deletes a file at given 'filepath'
 * 
 * @param filePath - the full path (root + filename) of the file to delete
 * @exception throws HTTPError when filepath is invalid, file not enabled for writing, or if syscall functions fail
 * @return (void)
 */
void Response::deleteFile( const std::string filePath ){
	std::cout << YELLOW << "Trying to delete FILE : " << filePath << RESET << std::endl;
// does the resource exists
	errno = 0;
    if (access(filePath.c_str(), F_OK) == -1){
        /*Not found*/	throw HttpError(404, matching_directives, take_location_root(matching_directives, false));

// do I have the right permission ? 
	errno = 0;
    if (access(filePath.c_str(), W_OK) == -1){
		if (errno == ETXTBSY) // ETXTBSY Write access was requested to an executable which is being executed.
		/*Conflict*/	throw HttpError(409, matching_directives, take_location_root(matching_directives, false));
		else
		/*Forbidden*/	throw HttpError(403, matching_directives, take_location_root(matching_directives, false));
	}
	}

// delete the file
	errno = 0;
    if (unlink(filePath.c_str()) == -1) {
		/*Server Err*/	throw HttpError(500, matching_directives, take_location_root(matching_directives, false));
	}
}

/**
 * @brief this function recursively deletes a directory and its subdirectories and files at a given 'directoryPath'
 * 
 * @param directoryPath - the full path (root + directoryname) of the directory to delete
 * @exception throws HTTPError when directoryPath is invalid, files not enabled for writing, or if syscall functions fail
 * @return (void)
 */
// readdir() MAN - RETURN VALUE 
//        On success, readdir() returns a pointer to a dirent structure. If the end of the directory stream is reached, NULL is returned
//        and errno is not changed.  If an error occurs, NULL is returned and errno is set to indicate the error.  To distinguish end of
//        stream from an error, set errno to zero before calling readdir() and then check the value of errno if NULL is returned.
void	Response::deleteDirectory(const std::string directoryPath)
{
	std::cout << YELLOW << "Trying to delete DIRECTORY : " << directoryPath << RESET << std::endl;
	const std::string	root = take_location_root(matching_directives, false);

	errno = 0;
    DIR* dir = opendir(directoryPath.c_str());
    if (dir){
        dirent* entry;
		errno = 0;
        while ((entry = readdir(dir)) != NULL){
			
			if (entry == NULL && errno != 0)								// errno, see above
				/*Server Err*/	throw HttpError(500, matching_directives, root);
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				continue;
			std::cout << CYAN << "| dir: " << directoryPath << "| entry : " << entry->d_name << ((entry->d_type == DT_DIR)? "is a directory" : "") << (((entry->d_type == DT_REG) || (entry->d_type == DT_LNK))? "is a regular file" : "") << ((!(entry->d_type == DT_DIR) && !(entry->d_type == DT_REG) && !(entry->d_type == DT_LNK))? "is neither a file nor a directory" : "") << RESET << std::endl;
			if (entry->d_type == DT_DIR)
				deleteDirectory(directoryPath + "/" + entry->d_name);
			else if ((entry->d_type == DT_REG) || (entry->d_type == DT_LNK))
				deleteFile(directoryPath + "/" + entry->d_name);
			else 
				/*Forbidden*/	throw HttpError(403, matching_directives, root);
        }
		errno = 0;
        if (closedir(dir) == -1)
			/*Server Err*/	throw HttpError(500, matching_directives, root);
		errno = 0;
        if (rmdir(directoryPath.c_str()) == -1) // error check is performed hereafter
			/*Server Err*/	throw HttpError(500, matching_directives, root);
    }
    else
		/*Server Err*/	throw HttpError(500, matching_directives, root, strerror(errno));
}


void Response::throw_HttpError_errno_stat(){
	const std::string root = take_location_root(matching_directives, false);

	switch(errno)
	{
	// 400 Bad Request
	case EFAULT:	throw HttpError(400, matching_directives, root, strerror(errno));  
	// 403 Forbidden
	case EACCES:	throw HttpError(403, matching_directives, root, strerror(errno)); 
	// 414 url too long
	case ENAMETOOLONG:throw HttpError(414, matching_directives, root, strerror(errno)); 
	// 500 Internal Server Error
	case ENOMEM:	throw HttpError(500, matching_directives, root, strerror(errno)); 
	case EOVERFLOW:	throw HttpError(500, matching_directives, root, strerror(errno)); 
	// 404 Not found
	case ELOOP :	throw HttpError(404, matching_directives, root, strerror(errno)); 
	case ENOENT :	throw HttpError(404, matching_directives, root, strerror(errno)); 
	case ENOTDIR:	throw HttpError(404, matching_directives, root, strerror(errno));  
	default:		throw HttpError(404, matching_directives, root, strerror(errno)); 
	}
}

// STAT ERROR
// EACCES|			Search permission is denied for one of the directories in the path prefix of path. (See also path_resolution(7).)
// EBADF|			fd is bad.
// EFAULT|			Bad address.
// ELOOP|			Too many symbolic links encountered while traversing the path.
// ENAMETOOLONG|	path is too long.
// ENOENT|			A component of path does not exist, or path is an empty string.
// ENOMEM|			Out of memory (i.e., kernel memory).
// ENOTDIR|			A component of the path prefix of path is not a directory.
// EOVERFLOW|		path or fd refers to a file whose size, inode number, or number of blocks cannot be represented in,
// 					respectively, the types off_t, ino_t, or blkcnt_t. 
// 					This error can occur when, for example, an application compiled on a 32-bit platform without 
// 					-D_FILE_OFFSET_BITS=64 calls stat() on a file whose size exceeds (1<<31)-1 bytes.



#include <cerrno>
#include <sys/stat.h>
#include <sys/types.h>

//_______________________ METHOD : DELETE _______________________
/**
 * @brief this function implements the DELETE method : delete a given file or directory and return a status of the operation
 * 
 * @param uri_path - the path of the file/directory to delete
 * @exception throws HTTPError when given file or directory is invalid for deleting, or syscall functions fail
 * @return (void)
 */

// A successful DELETE response SHOULD be 
	// 200 (OK) if the response includes an entity describing the status
	// 202 (Accepted) if the action has not yet been enacted
	// 204 (No Content) if the action has been enacted but the response does not include an entity.
// failed request
// 400 Bad Request: The request could not be understood or was malformed. The server should include information in the response body or headers about the nature of the error.
// 403 Forbidden: The server understood the request, but the client does not have permission to access the requested resource.
// 404 Not Found: The requested resource could not be found on the server.
// 414 URI Too Long : The URI requested by the client is longer than the server is willing to interpret.
// 409 Conflict: The request could not be completed due to a conflict with the current state of the resource. This is often used for data validation errors or when trying to create a resource that already exists.
// 500 Internal Server Error: An unexpected error occurred on the server, indicating a problem with the server's configuration or processing of the request.

void	Response::generateDELETEResponse( const std::string uri_path )
{
	const std::string				root = take_location_root(matching_directives, false);
	std::string						reqPath(uri_path);
	std::string						filePath;

    struct stat						fileStat;
	bool							is_dir;
	bool							is_reg;

	std::string						headers;
	std::vector<char>				body;
	std::string						tmp;	

	std::cout << "method: DELETE" << std::endl;
	std::cout << "uri_path : " << uri_path << std::endl;
	path_remove_leading_slash(reqPath);
	filePath = root + reqPath;
	std::cout << "filePath : " << filePath << std::endl;

	errno = 0;
    if (stat(filePath.c_str(), &fileStat) == 0) {
		is_dir = S_ISDIR(fileStat.st_mode);
		is_reg = S_ISREG(fileStat.st_mode);
		std::cout << MAGENTA << filePath <<" : " << (is_dir? "is a directory" : "") << (is_reg? "is a regular file" : "") << ((!is_dir && !is_reg)? "is neither a file nor a directory" : "") << RESET << std::endl;

		if (is_dir)
			deleteDirectory(filePath); 			// recursive call to delete content of directory (all files and subdirectories)
		else if (is_reg)
			deleteFile(filePath);
		else 									// existing resource that is not a regular file nor a directory
			/*Not allowed*/	throw HttpError(405, matching_directives, root, "url should point to a directory (POST)");
    }
	else 
		throw_HttpError_errno_stat(); 			// throws accurate http status code according to errno

// update the response
	tmp = (is_dir == true ? "Directory: " : "File: ") +  filePath  + " was successfully deleted\n";
	body.insert(body.begin(), tmp.begin(), tmp.end());
	headers = getHeaders(200, "OK", filePath, body.size());
	this->response.insert(this->response.begin(), headers.begin(), headers.end());
	this->response.insert(this->response.end(), body.begin(), body.end());
}

//_______________________ METHOD : POST _______________________
