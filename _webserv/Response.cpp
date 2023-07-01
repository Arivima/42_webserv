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

	// COUT_DEBUG_INSERTION("send_line()\n")
	if (response.empty())
		throw TaskFulfilled();
	else {
		if (NULL != eevent && eevent->events & EPOLLOUT)
		{
			bytes_sent = send(
				this->sock_fd, response.data(), response.size(), 0
			);
			if (bytes_sent < 0)
				throw HttpError(500, matching_directives, take_location_root(matching_directives, false));
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
			throw (std::runtime_error("not implemented"));
			this->cgi = new CGI(sock_fd, client_IP, server_IP, req, matching_directives, std::string(""));
			this->cgi->launch();
			this->response = this->cgi->getResponse();
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
{
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
		else
			throw HttpError(404, this->matching_directives, root);
	}
	else {
		COUT_DEBUG_INSERTION("serving page : " << root + reqPath << std::endl)
										filePath = root + reqPath;
		std::ifstream					docstream(filePath.c_str(), std::ios::binary);

		if (false == docstream.is_open()) {
			COUT_DEBUG_INSERTION("throwing page not found\n")
			throw HttpError(404, matching_directives, take_location_root(matching_directives, false));
		}
		try {
			page.insert(
				page.begin(),
				std::istreambuf_iterator<char>(docstream),
				std::istreambuf_iterator<char>());
		}
		catch (const std::exception& e) {
			COUT_DEBUG_INSERTION("throwing Internal Server Error\n")
			throw HttpError(500, matching_directives, take_location_root(matching_directives, false));
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
	size_t body_size
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
		<< "Content-Length : " << body_size
		<< "\r\n\r\n";
	
	return (headersStream.str());
}

std::string		Response::getIndexPage( const std::string& root, std::string path )
{
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
				return (path + cur_index);
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
{
	std::string		path = uri;
	size_t			path_start;

	//*	protection against telnet bad requests	(i.e.: when we write the http request by hand)
	if (
		std::string::npos == (path_start = uri.find("/"))
	)
	{
		throw HttpError(400, matching_directives, root);
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
// does the resource exists
	errno = 0;
    if (access(filePath.c_str(), F_OK) == -1){
		std::cout << "Response::deleteFile() access F_OK: 404" << std::endl;
        // throw HttpError(404, matching_directives, take_location_root(matching_directives, false));
	// 404 Not Found - The server cannot find the requested resource. In the browser, this means the URL is not recognized. 

// do I have the right permission ? 
	errno = 0;
    if (access(filePath.c_str(), W_OK) == -1){
		if (errno == ETXTBSY)
			std::cout << "Response::deleteFile() access W_OK: 409" << std::endl;
			// throw HttpError(409, matching_directives, take_location_root(matching_directives, false));
		else
			std::cout << "Response::deleteFile() access W_OK: 403" << std::endl;
			// throw HttpError(403, matching_directives, take_location_root(matching_directives, false));
	}
	// 401 Unauthorized - Although the HTTP standard specifies "unauthorized", semantically this response means "unauthenticated". 
		// That is, the client must authenticate itself to get the requested response.
	// 403 Forbidden - The client does not have access rights to the content; that is, it is unauthorized, so the server is refusing to give the requested resource. 
		// Unlike 401 Unauthorized, the client's identity is known to the server.
	// 409 Conflict: The deletion could not be completed due to a conflict with the current state of the resource. 
		// ETXTBSY		Write access was requested to an executable which is being executed.
	}

// unlink
	errno = 0;
    if (unlink(filePath.c_str()) == -1) {
		if ((errno == ELOOP) || (errno == ENOENT) || (errno == ENOTDIR) || (errno == EISDIR) || (errno == EFAULT))
			throw HttpError(400, matching_directives, take_location_root(matching_directives, false));
		else if ((errno == EACCES) || (errno == EPERM) || (errno == EROFS))
			throw HttpError(403, matching_directives, take_location_root(matching_directives, false));
		else if (errno == EBUSY)
			throw HttpError(409, matching_directives, take_location_root(matching_directives, false));
		else if (errno == ENAMETOOLONG)
			throw HttpError(414, matching_directives, take_location_root(matching_directives, false));
		else if ((errno == EIO) || (errno == ENOMEM))
			throw HttpError(500, matching_directives, take_location_root(matching_directives, false));
		else 
			throw HttpError(404, matching_directives, take_location_root(matching_directives, false));
	}
	// 404 Not Found 		- The server cannot find the requested resource. In the browser, this means the URL is not recognized. 
		// ??
	// 400 Bad Request 		- The server cannot or will not process the request due to something that is perceived to be a client error 
	// 						(e.g., malformed request syntax, invalid request message framing, or deceptive request routing).
		// ELOOP			Too many symbolic links were encountered in translating pathname.
		// ENOENT			A component in pathname does not exist or is a dangling symbolic link, or pathname is empty.
		// ENOTDIR			A component used as a directory in pathname is not, in fact, a directory.
		// EISDIR			pathname refers to a directory.  (This is the non-POSIX value returned since Linux 2.1.132.)
		// EFAULT			pathname points outside your accessible address space.

	// 403 Forbidden 		- The client does not have access rights to the content; that is, it is unauthorized, so the server is refusing to give the requested resource. 
	// 						Unlike 401 Unauthorized, the client's identity is known to the server.
		// EACCES			Write access to the directory containing pathname is not allowed for the process's effective UID, or one of the directories in pathname did not allow search permission.
		// EPERM (Linux)	The filesystem does not allow unlinking of files.
		// EPERM or EACCES	The directory containing pathname has the sticky bit (S_ISVTX) set and the process's effective UID is neither the UID of the file 
		// 					to be deleted nor that of the directory containing it, and the process is not privileged (Linux: does not have the CAP_FOWNER capability).
		// EPERM			The file to be unlinked is marked immutable or append-only.  (See ioctl_iflags(2).)	
		// EPERM  			The system does not allow unlinking of directories, or unlinking of directories requires privileges that the calling process doesn't have.
		// 					(This is the POSIX prescribed error return; as noted above, Linux returns EISDIR for this case.)
		// EROFS			pathname refers to a file on a read-only filesystem. 
		// 					The same errors that occur for unlink() and rmdir(2) can also occur for unlinkat().  The following additional errors can occur for unlinkat():


	// 409 Conflict			- The deletion could not be completed due to a conflict with the current state of the resource. 
		// EBUSY			The file pathname cannot be unlinked because it is being used by the system or another process; 
		//					for example, it is a mount point or the NFS client software created it to represent an active but otherwise nameless inode ("NFS silly renamed").

	// 414 URI Too Long		- The URI requested by the client is longer than the server is willing to interpret.
		// ENAMETOOLONG		pathname was too long.

	// 500 Internal Server Error: An unexpected error occurred on the server while processing the deletion request.
		// EIO				An I/O error occurred.
		// ENOMEM			Insufficient kernel memory was available.
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
	std::cout << MAGENTA << "deleteDirectory("<< directoryPath <<"): " << RESET << std::endl;

	errno = 0;
    DIR* dir = opendir(directoryPath.c_str());
    if (dir){
        dirent* entry;
		errno = 0;
        while ((entry = readdir(dir)) != NULL){
			
			if (entry == NULL && errno != 0)								// errno, see above
				throw HttpError(500, matching_directives, take_location_root(matching_directives, false));
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0){
				continue;
			}
			// std::cout << MAGENTA << "| dir: " << directoryPath << "| entry : " << entry->d_name << RESET << std::endl;
			if (entry->d_type == DT_DIR){
				std::cout << CYAN << "| dir: " << directoryPath << "| entry : " << entry->d_name << " is a directory " << RESET << std::endl;
				deleteDirectory(directoryPath + "/" + entry->d_name);
			}
			else if ((entry->d_type == DT_REG) || (entry->d_type == DT_LNK)){
				std::cout << MAGENTA << "| dir: " << directoryPath << "| entry : " << entry->d_name << " is a file " << RESET << std::endl;
				deleteFile(directoryPath + "/" + entry->d_name);
			}
			else {
				std::cout << RED << "| dir: " << directoryPath << "| entry : " << entry->d_name << " is nor a file nor a directory " << RESET << std::endl;
				throw HttpError(403, matching_directives, take_location_root(matching_directives, false));
			}
        }
		errno = 0;
        if (closedir(dir) == -1)
			throw HttpError(500, matching_directives, take_location_root(matching_directives, false));

		errno = 0;
        if (rmdir(directoryPath.c_str()) == -1) // error check is performed hereafter
			std::cout << "Debug : Response::deleteDirectory : ERR in RMDIR()" << std::endl;
    }
    if (errno != 0) {
		if ((errno == EACCES) || (errno == EPERM) || (errno == EROFS))
			throw HttpError(403, matching_directives, take_location_root(matching_directives, false));
		else if (errno == ENOENT)
			throw HttpError(404, matching_directives, take_location_root(matching_directives, false));
		else if (errno == EBUSY)
			throw HttpError(409, matching_directives, take_location_root(matching_directives, false));
		else{
			std::cout << "Debug : Response::deleteDirectory : unknown ERR in RMDIR() or opendir()" << std::endl;
			throw HttpError(500, matching_directives, take_location_root(matching_directives, false));
		}
	}
	// 403 Forbidden - The client does not have access rights to the content; that is, it is unauthorized, so the server is refusing to give the requested resource. 
	// Unlike 401 Unauthorized, the client's identity is known to the server.
		//    EACCES Permission denied.
		//	  EPERM  The filesystem containing pathname does not support the removal of directories.
		//	  EROFS  pathname refers to a directory on a read-only filesystem.
	// 404 Not Found - The server cannot find the requested resource. In the browser, this means the URL is not recognized. 
		//    ENOENT Directory does not exist, or directoryPath is an empty string.
	// 409 Conflict: The deletion could not be completed due to a conflict with the current state of the resource. 
		//    EBUSY pathname is currently in use by the system or some process that prevents its removal.
		// 	        On Linux, this means pathname is currently used as a mount point or is the root directory of the calling process.
	// 500 Internal Server Error: An unexpected error occurred on the server while processing the deletion request.
		//    EMFILE The per-process limit on the number of open file descriptors has been reached.
		//    ENFILE The system-wide limit on the total number of open files has been reached.
		//    ENOMEM Insufficient memory to complete the operation.
		//    ENOTDIR directoryPath is not a directory.
}

//_______________________ METHOD : DELETE _______________________
/**
 * @brief this function implements the DELETE method : delete a given file or directory and return a status of the operation
 * 
 * @param directoryPath - the full path (root + directoryname) of the directory to delete
 * @exception throws HTTPError when given file or directory is invalid for deleting, or syscall functions fail
 * @return (void)
 */
void	Response::generateDELETEResponse( const std::string uri_path )
{
	const std::string				root = take_location_root(matching_directives, false);
	std::string						reqPath(uri_path);
	std::string						headers;
	std::string						tmp;
	std::vector<char>				body;
	std::string						filePath;

	std::cout << "DELET uri_path : " << uri_path << std::endl;
	path_remove_leading_slash(reqPath);
	filePath = root + reqPath;
	std::cout << "DELETE full uri_path : " << filePath << std::endl;
	
// delete the file/directory
	if (true == isDirectory(root, reqPath, this->matching_directives)) {
		std::cout << YELLOW << "Trying to delete DIRECTORY : " << filePath << RESET << std::endl;
		deleteDirectory(filePath); // recursive call to delete content of directory (all files and subdirectories)
		tmp = "Directory \"" +  filePath  + "\" was successfully deleted\n";
		body.insert(body.begin(), tmp.begin(), tmp.end());
	}
	else {
		std::cout << YELLOW << "Trying to delete FILE : " << filePath << RESET << std::endl;
		deleteFile(filePath);
		tmp = "File \"" +  filePath  + "\" was successfully deleted\n";
		body.insert(body.begin(), tmp.begin(), tmp.end());
	}

// update the response
	headers = getHeaders(200, "OK", filePath, body.size());
	this->response.insert(this->response.begin(), headers.begin(), headers.end());
	this->response.insert(this->response.end(), body.begin(), body.end());

// A successful response SHOULD be 
	// 200 (OK) if the response includes an entity describing the status
	// 202 (Accepted) if the action has not yet been enacted
	// 204 (No Content) if the action has been enacted but the response does not include an entity.
}

//_______________________ METHOD : POST _______________________
