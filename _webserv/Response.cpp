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
			return (generateGETResponse(uri_remove_queryString(req.at("url"))));
		// if ("POST" == this->req.at("method"))
		// 	return (generatePOSTResponse(uri_remove_queryString(req.at("url"))));
		// if ("PUT" == this->req.at("method"))
		// 	return (generatePUTResponse());
		// if ("DELETE" == this->req.at("method"))
		// 	return (generateDELETEResponse());
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
