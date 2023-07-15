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

#include "Response.hpp"
# include "Utils.hpp"

#include <iostream>
#include <fstream>		//open requested files
#include <sstream>		//stringstream
#include <string>
#include <cstring>		//memeset
#include <list>			//location matches sorting

#include <unistd.h>		// access, unlink
#include <stdio.h>		// remove
#include <sys/socket.h>	//send
#include <sys/stat.h>	// stat
#include <sys/types.h>
#include <dirent.h>
#include <cerrno>		// errno


//*		Main Constructor and Destructor
Response::Response(
	Request*			request,
	const t_server&		assigned_server,
	const int			sock_fd,
	const std::string&	client_IP,
	const std::string&	server_IP,
	const t_epoll_data&	edata
)
		:
		matching_directives(
			takeMatchingDirectives(
				assigned_server.conf_server_block,
				request->getRequest()
			)
		),
		request(request),
		req(request->getRequest()),
		location_root(take_location_root()),
		assigned_server(assigned_server),
		sock_fd(sock_fd),
		client_IP(client_IP),
		server_IP(server_IP),
		edata(edata),
		uri_path(uri_remove_queryString(req.at("url")))
{
	dechunking = false;
	chunk_bytes_sent = 0;
	redirect = (
		matching_directives.directives.end() !=
		matching_directives.directives.find("return")
	);
	this->cgi = NULL;
}

Response::~Response() {
	response.clear();
	if (this->cgi)
		delete (this->cgi);
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
			bytes_sent = send(
				this->sock_fd, response.data(), response.size(), 0
			);
			if (bytes_sent < 0)
				/*Server Err*/	throw HttpError(500, matching_directives, location_root);
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

void	Response::handle_next_chunk( void )
{
	try {
		if (this->cgi)
			this->cgi->CGINextChunk();
		else
			this->POSTNextChunk();
	}
	catch (const TaskFulfilled& e) {
		dechunking = false;
		this->response = cgi->getResponse();
		this->print_resp();
	}
	catch (const HttpError& e) {
		dechunking = false;
		std::cout << RED << e.what() << RESET << std::endl;
		this->response = e.getErrorPage();
		this->print_resp();
	}
}

void	Response::POSTNextChunk( void )
{ COUT_DEBUG_INSERTION(YELLOW "Response::POSTNextChunk()" RESET << std::endl);
	std::vector<char>	incomingData;
	std::string			root = location_root;
	std::string			reqPath(uri_path);

	try {
		try
		{
			incomingData = request->getIncomingData();
		}
		catch (const ChunkNotComplete& e) {
			return ;
		}
		catch (const std::invalid_argument& e) {//*invalid chunk
			stream_newFile.close();
			unlink(newFileName.c_str());
			/*Bad req*/	throw (HttpError(400, matching_directives, location_root));
		}

		if (incomingData.empty())//*last chunk
		{
			//*	closing newly created file
			stream_newFile.close();
			if (stream_newFile.fail()) {
				unlink(newFileName.c_str());
				/*Server Err*/	throw HttpError(500, this->matching_directives, root);
			}

			//* update the response
			std::string						location_header;
			std::string						headers;
			std::string						tmp;
			std::string						body;
			std::string						domainName;
			std::string						newResourceUrl;
			std::string						newResourceRelPath;

			domainName		= std::string(server_IP) + ":" + matching_directives.directives.at("listen");
			newResourceUrl	= "http://" + domainName + "/" + newFileDir + "/" + newFileName;
			newResourceRelPath = newFileDir + "/" + newFileName;

			body				= "Resource " +  newResourceUrl  + " at directory\"" +  newFileDir  + "\" was successfully created\n";
			location_header	= std::string("Location: " + newResourceUrl);
			headers			= getHeaders(201, "OK", newResourceRelPath, body.size(), location_header);

			dechunking = false;
			this->response.insert(this->response.begin(), headers.begin(), headers.end());
			this->response.insert(this->response.end(), body.begin(), body.end());
			this->print_resp();
		}
		else
		{
			//*	printing next chunk bytes into body
			stream_newFile.write(incomingData.data(), incomingData.size());
			if (
				stream_newFile.tellp() > std::atol(matching_directives.directives.at("body_size").c_str())
			) {
				stream_newFile.close();
				unlink((root + "/" + newFileDir + "/" + newFileName).c_str());
				/*Content Too Large*/	throw HttpError(413, matching_directives, location_root);
			}
			if (stream_newFile.fail()) {
				stream_newFile.close();
				unlink(newFileName.c_str());
				/*Server Err*/	throw HttpError(500, this->matching_directives, root);
			}
		}
	}
	catch (const HttpError& e) {
		this->dechunking = false;
		this->response = e.getErrorPage();
		this->print_resp();
	}
}

bool	Response::isDechunking(void)
{
	return (this->dechunking);
}

bool	Response::isRedirect( void )
{
	return (this->redirect);
}

void	Response::generateResponse( void )
{
	std::string		cgi_extension = take_cgi_extension(
						req.at("url"), matching_directives.directives
					);
	std::string		url_no_query_str = uri_remove_queryString(req.at("url"));

	try
	{
		COUT_DEBUG_INSERTION( BOLDGREEN "generateResponse() for : " RESET << url_no_query_str << std::endl);
		if (request->timedOut()) {
			/*Gateway Timeout*/	throw (HttpError(504, matching_directives, location_root));
		}
		if (std::string::npos != req.at("url").find("/..")) {//*input sanitization
			/*Bad req*/	throw HttpError(400, matching_directives, location_root);
		}
		if (redirect) {
			return (handle_redirection(matching_directives.directives.at("return")));
		}
		if (false == cgi_extension.empty())
		{
			if (this->request->isChunked())
				this->dechunking = true;
			return (generateCGIResponse(cgi_extension));
		}
		if ("GET" == this->req.at("method")){
			std::cout << BOLDMAGENTA << "METHOD : GET" << RESET << std::endl;
			return (generateGETResponse());
		}
		if ("POST" == this->req.at("method"))
		{
			std::cout << BOLDMAGENTA << "METHOD : POST" << RESET << std::endl;
			if (this->request->isChunked()) {
				this->dechunking = true;
				return (generateChunkedPOSTResponse());
			}
			return (generatePOSTResponse());
		}
		if ("DELETE" == this->req.at("method")) {
			std::cout << BOLDMAGENTA << "METHOD : DELETE" << RESET << std::endl;
			return (generateDELETEResponse());
		}
		
		/*Not Implemented*/	throw (HttpError(501, this->matching_directives, location_root));
	}
	catch (const HttpError& e) {
		this->dechunking = false;
		this->response = e.getErrorPage();
		this->print_resp();
	}
}



//_______________________ METHOD : GET _______________________
// refactor to clean + test and make sure all cases are covered
void	Response::generateGETResponse(  void  )
{COUT_DEBUG_INSERTION(YELLOW "Response::generateGETResponse()" RESET << std::endl);

	const std::string				root = location_root;
	const std::string				reqPath(http_req_complete_url_path(uri_path, root));
	std::string						headers;
	std::vector<char>				page;
	std::string						filePath = "";

	COUT_DEBUG_INSERTION("Path of the request : " << (reqPath.empty()? "EMPTY" : reqPath ) << std::endl);
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
				getDirectoryContentList(root + path) //!wip
			);
			page.insert(
				page.begin(),
				dir_listing_page.begin(),
				dir_listing_page.end()
			);
			headers = getHeaders(200, "OK", filePath, page.size());
		}
		else {
			COUT_DEBUG_INSERTION("autoindex not set" << std::endl);
			/*Forbidden*/	throw HttpError(403, this->matching_directives, root);
		}
	}
	else {
		COUT_DEBUG_INSERTION("serving page : " << root + reqPath << std::endl)
										filePath = root + reqPath;
		std::ifstream					docstream(filePath.c_str(), std::ios::binary);

		check_file_accessibility(
			R_OK,
			reqPath, root,
			matching_directives
		);
		if (false == docstream.is_open()) {
			COUT_DEBUG_INSERTION("could not open file (server error)\n");
			/*Server Err*/	throw HttpError(500, matching_directives, location_root);
		}
		try {
			page.insert(
				page.begin(),
				std::istreambuf_iterator<char>(docstream),
				std::istreambuf_iterator<char>());
		}
		catch (const std::exception& e) {
			COUT_DEBUG_INSERTION("IO file corruption\n");
			/*Server Err*/	throw HttpError(500, matching_directives, location_root);
		}
		headers = getHeaders(200, "OK", filePath, page.size());
	}
	response.insert(response.begin(), headers.begin(), headers.end());
	response.insert(response.end(), page.begin(), page.end());
}

//_______________________ METHOD : POST _______________________
void	Response::generatePOSTResponse( void )
{
	std::string						root = location_root;
	std::string						reqPath(uri_path);
	std::string						newFileDir;
	std::string						newFileName;
	std::string						fileExtension;
	std::string						fullDirPath;
	std::string						fullFilePath;

	COUT_DEBUG_INSERTION("POST uri_path : " << uri_path << std::endl);


	// check if existing filename and splits reqPath into dir and newFileName
	size_t pos = reqPath.rfind("/");
	if (pos != std::string::npos && pos < reqPath.length() - 1){//if / exists and there's characters after it
		newFileName = reqPath.substr(pos + 1);
	}
	else {// if no filename given
		newFileName = "test_POST";
	}
	newFileDir = reqPath.substr(0, pos);
	path_remove_leading_slash(newFileDir);
	path_remove_leading_slash(root);
	path_remove_leading_slash(reqPath);
	
	fullDirPath		= root + (newFileDir.empty() ? "" : "/" + newFileDir);
	fullFilePath	= root + (newFileDir.empty() ? "" : "/" + newFileDir) + (newFileName.empty() ? "" : "/" + newFileName);

	COUT_DEBUG_INSERTION("POST reqPath : "	 	<< reqPath << std::endl);
	COUT_DEBUG_INSERTION("POST root : "	 		<< root << std::endl);
	COUT_DEBUG_INSERTION("POST dir : "	 		<< newFileDir << std::endl);
	COUT_DEBUG_INSERTION("POST newFileName : "	<< newFileName << std::endl);
	COUT_DEBUG_INSERTION("POST fullDirPath : "	<< fullDirPath << std::endl);
	COUT_DEBUG_INSERTION("POST fullFilePath : "	<< fullFilePath << std::endl);

// checks if fullDirPath is pointing to a valid location (directory)
    struct stat						fileStat;
	size_t							extension_pos;

	errno = 0;
	if (isDirectory(root, newFileDir))
	{
			if (false == isMethodAllowed()) {
				/*Not Allowed*/	throw (HttpError(405, matching_directives, location_root));
			}
			if (false == check_body_size()) {
				/*Content Too Large*/	throw HttpError(413, this->matching_directives, location_root);
			}
			// create the new resource at the location
			COUT_DEBUG_INSERTION(MAGENTA << "Trying to add a resource to DIRECTORY : " << fullDirPath << RESET << std::endl);

			// check if file exists at the given location and updating the name if it does
			extension_pos = newFileName.rfind(".");
			if (std::string::npos != extension_pos)
				fileExtension = newFileName.substr(extension_pos);
			else
				fileExtension = "";
			errno = 0;
			while (stat(fullFilePath.c_str(), &fileStat) == 0) {
				if (fileExtension.empty() == false)
					newFileName	= newFileName.substr(0, newFileName.rfind(fileExtension));
				newFileName += "_cpy" + fileExtension;
				if (newFileName.size() >= 255)
					/*Server Err*/	throw HttpError(500, this->matching_directives, root);
				fullFilePath	= root + "/" + newFileDir + "/" + newFileName;
			}
			if (errno && ENOENT != errno)
				throw return_HttpError_errno_stat(root, matching_directives);
			
			// writes body to new file
			std::ofstream	stream_newFile(fullFilePath);
			if (false == stream_newFile.is_open())
				/*Server Err*/	throw HttpError(500, this->matching_directives, root);
			
			stream_newFile.write(request->getPayload().data(), request->getPayload().size());
			if (stream_newFile.fail()) {
				stream_newFile.close();
				/*Server Err*/	throw HttpError(500, this->matching_directives, root);
			}
			stream_newFile.close();
	}
	else if (errno)
	{
		throw return_HttpError_errno_stat(root, matching_directives);
	}
	else
	{
		/*Not allowed*/	throw HttpError(405, this->matching_directives, root, "url should point to a directory (POST)");
	}

// update the response
	std::string						location_header;
	std::string						headers;
	std::string						tmp;
	std::string						body;
	std::string						domainName;
	std::string						newResourceUrl;
	std::string						newResourceRelPath;

	domainName		= std::string(server_IP) + ":" + matching_directives.directives.at("listen");
	newResourceUrl	= "http://" + domainName + "/" + newFileDir + "/" + newFileName;
	newResourceRelPath = newFileDir + "/" + newFileName;

	body				= "Resource " +  newResourceUrl  + " at directory\"" +  newFileDir  + "\" was successfully created\n";
	location_header	= std::string("Location: " + newResourceUrl);
	headers			= getHeaders(201, "OK", newResourceRelPath, body.size(), location_header);
	
	this->response.insert(this->response.begin(), headers.begin(), headers.end());
	this->response.insert(this->response.end(), body.begin(), body.end());
}

//_______________________ METHOD : POST Chunked _______________________
void	Response::generateChunkedPOSTResponse( void )
{
	std::string						root = location_root;
	std::string						reqPath(uri_path);
	std::string						fullFilePath;
	std::string						fullDirPath;
	std::string						fileExtension;

	COUT_DEBUG_INSERTION("POST uri_path : " << uri_path << std::endl);

	path_remove_leading_slash(root);
	path_remove_leading_slash(reqPath);

	// check if existing filename and splits reqPath into dir and newFileName
	size_t pos = reqPath.rfind("/");
	if (pos != std::string::npos && pos < reqPath.length() - 1){//if / exists and there's characters after it
		newFileName = reqPath.substr(pos + 1);
	}
	else {// if no filename given
		newFileName = "test_POST";
	}
	newFileDir = reqPath.substr(0, pos);
	
	fullDirPath		= root + "/" + newFileDir;
	fullFilePath	= root + "/" + newFileDir + "/" + newFileName;

	COUT_DEBUG_INSERTION("POST reqPath : "	 	<< reqPath << std::endl);
	COUT_DEBUG_INSERTION("POST root : "	 		<< root << std::endl);
	COUT_DEBUG_INSERTION("POST dir : "	 		<< newFileDir << std::endl);
	COUT_DEBUG_INSERTION("POST newFileName : "	<< newFileName << std::endl);
	COUT_DEBUG_INSERTION("POST fullDirPath : "	<< fullDirPath << std::endl);
	COUT_DEBUG_INSERTION("POST fullFilePath : "	<< fullFilePath << std::endl);

// checks if fullDirPath is pointing to a valid location (directory)
    struct stat						fileStat;
	size_t							extension_pos;

	errno = 0;
	if (isDirectory(root, newFileDir))
	{
			if (false == isMethodAllowed()) {
				/*Not Allowed*/	throw (HttpError(405, matching_directives, location_root));
			}
			// create the new resource at the location
			COUT_DEBUG_INSERTION(MAGENTA << "Trying to add a resource to DIRECTORY : " << fullDirPath << RESET << std::endl);

			// check if file exists at the given location and updating the name if it does
			extension_pos = newFileName.rfind(".");
			if (std::string::npos != extension_pos)
				fileExtension = newFileName.substr(extension_pos);
			else
				fileExtension = "";
			errno = 0;
			while (stat(fullFilePath.c_str(), &fileStat) == 0) {
				if (fileExtension.empty() == false)
					newFileName	= newFileName.substr(0, newFileName.rfind(fileExtension));
				newFileName += "_cpy" + fileExtension;
				if (newFileName.size() >= 255)
					/*Server Err*/	throw HttpError(500, this->matching_directives, root);
				fullFilePath	= root + "/" + newFileDir + "/" + newFileName;
				errno = 0;
			}
			if (errno && ENOENT != errno)
				throw return_HttpError_errno_stat(root, matching_directives);
			
			// writes body to new file
			stream_newFile.open(fullFilePath);
			if (false == stream_newFile.is_open())
				/*Server Err*/	throw HttpError(500, this->matching_directives, root);
	}
	else if (errno)
	{
		throw return_HttpError_errno_stat(root, matching_directives);
	}
	else
	{
		/*Not allowed*/	throw HttpError(405, this->matching_directives, root, "url should point to a directory (POST)");
	}
}

//_______________________ METHOD : DELETE _______________________
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
/**
 * @brief this function implements the DELETE method : delete a given file or directory and return a status of the operation
 * 
 * @param uri_path - the path of the file/directory to delete
 * @exception throws HTTPError when given file or directory is invalid for deleting, or syscall functions fail
 * @return (void)
 */
void	Response::generateDELETEResponse( void )
{
	const std::string				root = location_root;
	std::string						reqPath(uri_path);
	std::string						filePath;

    struct stat						fileStat;
	bool							is_dir;
	bool							is_reg;

	std::string						headers;
	std::vector<char>				body;
	std::string						tmp;	

	COUT_DEBUG_INSERTION("uri_path : " << uri_path << std::endl);
	path_remove_leading_slash(reqPath);
	filePath = root + reqPath;
	COUT_DEBUG_INSERTION("filePath : " << filePath << std::endl);

	errno = 0;
	if (stat(filePath.c_str(), &fileStat) == 0) {
		if (false == isMethodAllowed()) {
			/*Not Allowed*/	throw (HttpError(405, matching_directives, location_root));
		}
		is_dir = S_ISDIR(fileStat.st_mode);
		is_reg = S_ISREG(fileStat.st_mode);
		COUT_DEBUG_INSERTION(
			MAGENTA 
			<< filePath <<" : " 
			<< (is_dir? "is a directory" : "") 
			<< (is_reg? "is a regular file" : "") 
			<< ((!is_dir && !is_reg)? "is neither a file nor a directory" : "") 
			<< RESET 
			<< std::endl);

		if (is_dir)
			deleteDirectory(filePath); 			// recursive call to delete content of directory (all files and subdirectories)
		else if (is_reg)
			deleteFile(filePath);
		else 									// existing resource that is not a regular file nor a directory
			/*Not allowed*/	throw HttpError(405, matching_directives, root, "url should point to a directory (POST)");
    }
	else {
		COUT_DEBUG_INSERTION(YELLOW"Response::generateDELETEResponse()---stat failed" RESET << std::endl);
		throw return_HttpError_errno_stat(location_root, matching_directives);
	}

// update the response
	tmp = (is_dir == true ? "Directory: " : "File: ") +  filePath  + " was successfully deleted\n";
	body.insert(body.begin(), tmp.begin(), tmp.end());
	headers = getHeaders(200, "OK", filePath, body.size());
	this->response.insert(this->response.begin(), headers.begin(), headers.end());
	this->response.insert(this->response.end(), body.begin(), body.end());
}

//_______________________ CGI - METHOD : ANY _______________________
void	Response::generateCGIResponse(const std::string& cgi_extension)
{
	std::string		cgi_interpreter_path;

	COUT_DEBUG_INSERTION(GREEN << "Response::generateResponse there is a CGI extension" << RESET << std::endl);
	
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
	this->cgi = new CGI(
		sock_fd, client_IP, server_IP,
		request, dechunking,
		matching_directives, location_root,
		cgi_extension, cgi_interpreter_path
	);
	check_file_accessibility(
		X_OK,
		this->cgi->get_env_value("INTERPRETER_PATH"), "",
		matching_directives
	);
	check_file_accessibility(
		R_OK,
		this->cgi->get_env_value("SCRIPT_NAME"),
		this->cgi->get_env_value("ROOT"),
		matching_directives
	);
	if (false == isMethodAllowed()) {
		/*Not Allowed*/	throw (HttpError(405, matching_directives, location_root));
	}
	if (false == dechunking && false == check_body_size()) {
		/*Content Too Large*/	throw HttpError(413, this->matching_directives, location_root);
	}
	this->cgi->launch();
	if (false == dechunking)
	{
		this->response = this->cgi->getResponse();

		std::string	debug(this->response.begin(), this->response.end());
		COUT_DEBUG_INSERTION(
			"CGI response : " << std::endl
			<< "|" << debug << "|" << std::endl;
		);
	}
	
	return ;
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

std::string		Response::take_location_root( void )
{COUT_DEBUG_INSERTION(YELLOW "Response::take_location_root()" RESET << std::endl);
	std::string											directive;
	std::string											root;
	std::map<std::string, std::string>::const_iterator	root_pos;

	if (
		"POST" == this->req.at("method") &&
		matching_directives.directives\
			.end() != matching_directives.directives.find("upload_path")
	) {
		directive = "upload_path";
	}
	else {
		directive = "root";
	}
	root_pos = matching_directives.directives.find(directive);
	if (matching_directives.directives.end() != root_pos) {
		root = matching_directives.directives.at(directive);
		path_remove_leading_slash(root);
		root += "/";
	}
	else
		root = "./";
	
	COUT_DEBUG_INSERTION(YELLOW "END------Response::take_location_root()" RESET << std::endl);
	return (root);
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



//*		Secondary Helper Functions

bool			Response::isMethodAllowed(void)
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

// check request against body size value, else throw 413 Content Too Large
	// value validity is checked in config
	// body_size always exists as its default value (1M) was set in value validity checking
bool	Response::check_body_size()
{
	if (this->req.find("Content-Length") != this->req.end())
	{
		size_t req_body_size = std::atol(this->req.at("Content-Length").c_str());
		size_t max_body_size = std::atol(this->matching_directives.directives.at("body_size").c_str());
		
		COUT_DEBUG_INSERTION(
			GREEN
			<< "Content-Length : " << this->req.at("Content-Length") << std::endl
			<< "body_size : " << this->matching_directives.directives.at("body_size") << std::endl
			<< "req_body_size : " << req_body_size << std::endl
			<< "max_body_size : " << max_body_size << std::endl
			<< RESET << std::endl
		);

		return (req_body_size <= max_body_size);
	}
	return (true);
}

// validity of configuration has been checked in the parsing
	// code can be 307 or 308
	// URL is validity is not checked -> responsibility of admin
// 307	Temporary redirect		- Method and body not changed 
							//	- The Web page is temporarily unavailable for unforeseen reasons.
							//	- Better than 302 when non-GET operations are available on the site.
// 308	Permanent Redirect
							//	- Method and body not changed.
							//	- Reorganization of a website, with non-GET links/operations.
void	Response::handle_redirection(const std::string & value)
{ COUT_DEBUG_INSERTION(YELLOW "Response::handle_redirection()" RESET << std::endl);
	// get the redirection information from the matching directives
    std::istringstream	iss(value);
	std::string			http_status_code;
	std::string			redirection_url;
	const std::string	location = matching_directives.directives.at("location");
	std::string			file_name;

	if (location.length() + 1 < uri_path.length())
		file_name = uri_path.substr(location.length());
	else
		file_name = "";
    iss >> http_status_code;
    iss >> redirection_url;
	redirection_url += file_name;

	strip_trailing_and_leading_spaces(redirection_url);
	strip_trailing_and_leading_spaces(http_status_code);
	COUT_DEBUG_INSERTION(
		GREEN
		<< "http_status_code : " << http_status_code << std::endl
		<< "redirection_url : " << redirection_url << std::endl
		<< RESET
		<< std::endl
	);

	// update the response
	std::stringstream				headersStream;
	std::string						headers;

	headersStream
		<< "HTTP/1.1 " << http_status_code << " " << (http_status_code == "307" ? "Temporary redirect" : "Permanent Redirect") << "\r\n"
		<< std::string("Location: " + redirection_url) + "\r\n"
		<< "\r\n";
	
	headers = headersStream.str();
	this->response.insert(this->response.begin(), headers.begin(), headers.end());
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
	errno = 0;
	if ( true == isDirectory( root, path) ) {
		path = getIndexPage(root, path);
	}
	else if (errno) {
		throw return_HttpError_errno_stat(location_root, matching_directives);
	}
	// else path refers to a regular file and is already correct

	path_remove_leading_slash(path);
	return (path);//*	may be empty (a.k.a. "")
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

/**
 * @brief this function deletes a file at given 'filepath'
 * 
 * @param filePath - the full path (root + filename) of the file to delete
 * @exception throws HTTPError when filepath is invalid, file not enabled for writing, or if syscall functions fail
 * @return (void)
 */
void			Response::deleteFile( const std::string filePath )
{
	COUT_DEBUG_INSERTION(YELLOW << "Trying to delete FILE : " << filePath << RESET << std::endl);

	check_file_deletable(filePath, "", matching_directives);
// delete the file
	errno = 0;
    if (unlink(filePath.c_str()) == -1) {
		COUT_DEBUG_INSERTION(RED "Response::deleteFile() : unlink error" RESET << std::endl);
		/*Server Err*/	throw HttpError(500, matching_directives, location_root);
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
void			Response::deleteDirectory(const std::string directoryPath)
{
	COUT_DEBUG_INSERTION(YELLOW << "Trying to delete DIRECTORY : " << directoryPath << RESET << std::endl);
	const std::string	root = location_root;

	check_directory_deletable(directoryPath, "", matching_directives);
	errno = 0;
    DIR* dir = opendir(directoryPath.c_str());
    if (dir){
        dirent* entry;
        while (true)
		{
			errno = 0;
			entry = readdir(dir);
			try {
				if (entry == NULL && errno != 0)								// errno, see above
					/*Server Err*/	throw HttpError(500, matching_directives, root);
				else if (entry == NULL)
					break ;
            	if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
					continue;

				COUT_DEBUG_INSERTION(
					CYAN 
					<< "| dir: " << directoryPath 
					<< "| entry : " << entry->d_name 
					<< ((entry->d_type == DT_DIR)? "is a directory" : "") 
					<< (((entry->d_type == DT_REG) || (entry->d_type == DT_LNK))? "is a regular file" : "") 
					<< ((!(entry->d_type == DT_DIR) && !(entry->d_type == DT_REG) && !(entry->d_type == DT_LNK))? "is neither a file nor a directory" : "") 
					<< RESET 
					<< std::endl);

				if (entry->d_type == DT_DIR)
					deleteDirectory(directoryPath + "/" + entry->d_name);
				else if ((entry->d_type == DT_REG) || (entry->d_type == DT_LNK))
					deleteFile(directoryPath + "/" + entry->d_name);
				else 
					/*Forbidden*/	throw HttpError(403, matching_directives, root);
			}
			catch (const HttpError& e) {
				closedir(dir);
				throw (e);
			}
        }
		errno = 0;
        if (closedir(dir) == -1)
			/*Server Err*/	throw HttpError(500, matching_directives, root);
		errno = 0;
        if (rmdir(directoryPath.c_str()) == -1) // error check is performed hereafter
			/*Server Err*/	throw HttpError(500, matching_directives, root);
    }
    else {
		COUT_DEBUG_INSERTION(RED "Response::deleteDirectory() : could not open dir error" RESET << std::endl);
		/*Server Err*/	throw HttpError(500, matching_directives, root, strerror(errno));
	}
}



//*		Minor utils
void	Response::print_resp( void )
{
	std::cout	<< BOLDCYAN "\nNew Response ----------------- " << RESET 
				<< CYAN " | cli_socket: " << this->sock_fd << RESET
				<< " | lenght: " << response.size() 
				<< std::endl ;

	std::cout << CYAN "|" RESET;
	std::cout.write(response.data(), response.size());
	std::cout << CYAN "|" RESET;
	std::cout << std::endl;
	
	std::cout << CYAN "END Response -----------------" RESET << std::endl;
}
