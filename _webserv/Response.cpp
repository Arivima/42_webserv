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

//*		non-member helper functions
//*		/////////////////////////////////////////////////

Response::Response(
	const std::map<std::string, std::string>& req,
	const t_server& assigned_server,
	const int sock_fd,
	const t_epoll_data& edata)
		:
		matching_directives(takeMatchingDirectives(assigned_server.conf_server_block, req)),
		req(req),
		assigned_server(assigned_server),
		sock_fd(sock_fd),
		edata(edata)
{
}

Response::~Response() {
	response.clear();
}

//TODO check that the request method is inside the list of accepted method
//TODO inside the conf block (i.e.: "method" directive)
void	Response::generateResponse( void ) {
	try {
		if ("GET" == this->req.at("method"))
			return (generateGETResponse());
		// if ("POST" == this->req.at("method"))
		// 	return (generatePOSTResponse());
		// if ("PUT" == this->req.at("method"))
		// 	return (generatePUTResponse());
		// if ("DELETE" == this->req.at("method"))
		// 	return (generateDELETEResponse());
		throw (std::runtime_error("Response::generateResponse() : case not yet implemented"));
		// 501 Not Implemented
		// The request method is not supported by the server and cannot be handled. 
		// The only methods that servers are required to support (and therefore that must not return this code) are GET and HEAD.
	}
	catch (const HttpError& e) {
		this->response = e.getErrorPage();
	}
}

//TODO	TOMORROW
//TODO
//TODO	1.	refactor whole ConnectionSocket, make Request & Response classes
//TODO	2.	each class should throw an exception when task is complete so that ConnectionSocket
//TODO	can switch state with its switch-state functions
//TODO		2.1 make only one switch-state function, that switches to response or request
//TODO			depending on current status
//TODO	3.	when the ConnectionSocket catches a fulfillment exception, it should call the
//TODO		serve_client function again recursively (there will never be two consecutive
//TODO		recursive calls)
//*		....
//*		Done
//TODO	3.	make response field in Response class a string
//TODO		and use bytes_read to get position of first unsent byte, then trim the string
//TODO	4.	use SND_BUFFER_SIZE
//TODO
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
				throw HttpError(500, matching_directives, take_location_root());
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

// refactor to clean + test and make sure all cases are covered
void	Response::generateGETResponse( void )
{
	const std::string				root
		= take_location_root();
	const std::string				reqPath(
		http_req_take_url_path(req.at("url"), root)
	);;
	std::string						headers;
	std::vector<char>				page;
	std::string						filePath = "";

	//TODO autoindex (CHECK that path refers to directory and not a regular file)
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
			throw HttpError(404, matching_directives, take_location_root());
		}
		try {
			page.insert(
				page.begin(),
				std::istreambuf_iterator<char>(docstream),
				std::istreambuf_iterator<char>());
		}
		catch (const std::exception& e) {
			COUT_DEBUG_INSERTION("throwing Internal Server Error\n")
			throw HttpError(500, matching_directives, take_location_root());
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

//*		helper functions

//TODO	TEST 
bool	Response::locationMatch(
	const t_conf_block& location, const std::string& req_url
	)
{
	size_t		path_begin;
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
		return (
			0 == reqUrl.find("http://") &&
			std::string::npos != (path_begin = reqUrl.substr(7).find("/")) &&
			0 == reqUrl.substr(7 + path_begin).compare(location_path)
		);
	}
	else
	{
		//*		prefix match
		location_path = location.directives.at("location");
		strip_trailing_and_leading_spaces(location_path);
		return (
			0 == reqUrl.find("http://") &&
			std::string::npos != (path_begin = reqUrl.substr(7).find("/")) &&
			0 == reqUrl.substr(7 + path_begin).find(location_path)
		);
	}
}

/**
 * @brief This function returns a handle to a generic block holding directives.
 * Given a server block, it returns the inner location block that matches
 * the current request, or the server block itself if no location block matches.
 * @param conf_server_block 
 * @return const t_conf_block& 
 */
const t_conf_block&	Response::takeMatchingDirectives(
	const t_conf_block& conf_server_block,
	const std::map<std::string, std::string>& req
	)
{
	const t_conf_block&							virtual_server 
		= takeMatchingServer(conf_server_block.sub_blocks, req);
	std::vector<t_conf_block>::const_iterator	location;
	
	for (
		location = virtual_server. sub_blocks.begin();
		location != virtual_server.sub_blocks.end();
		location ++
	)
	{
		if (locationMatch(*location, req.at("url")))
			break ;
	}

	if (virtual_server.sub_blocks.end() == location)
		return (virtual_server);
	else
		return (*location);
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

//*		Private Member Helper functions

//*	(la funzione http_req_take_url_path() deve controllare che il path sia una directory;
//*	se lo è, deve tornare la getIndexPage().
//*	Se non è settata la directive index oppure le pagine indicate non esistono,
//*	getIndexPage() ritorna vuoto.
//*	Nel caso di reqPath vuota, generateGETResponse() deve gestire directive autoindex)
std::string		Response::http_req_take_url_path(
	const std::string& url, const std::string& root
	)
{
	std::string		path;
	size_t			path_start;
	size_t			path_end;

	if (
		std::string::npos == (path_start = url.find("/"))
	)
	{
		COUT_DEBUG_INSERTION("throwing url : " << url << std::endl)
		throw HttpError(400, matching_directives, take_location_root());
	}

	//*		remove query string
	path_end = url.rfind("?");
	if (std::string::npos == path_end)
		path_end = url.length();
	
	path = url.substr(path_start, path_end - path_start);

	if ( true == isDirectory( root, path, this->matching_directives) ) {
		path = getIndexPage(root, path);
	}
	// else path refers to a file and is already correct

	path_remove_leading_slash(path);
	return (path);//*	may be empty (a.k.a. "")
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

std::string		Response::take_location_root( void )
{
	std::string											root;
	std::map<std::string, std::string>::const_iterator	root_pos
		= matching_directives.directives.find("root");

	if (matching_directives.directives.end() != root_pos) {
		root = matching_directives.directives.at("root");
		path_remove_leading_slash(root);
		root += "/";
	}
	else
		root = "./";
	
	return (root);
}


POST /test/demo_form.php HTTP/1.1
Host: w3schools.com

name1=value1&name2=value2



POST /api/endpoint HTTP/1.1
Host: example.com
Content-Type: application/json
Content-Length: 26

{
  "key1": "value1",
  "key2": "value2"
}


void	Response::generatePOSTResponse( void )
{
	std::cout << "Response::generatePOSTResponse" << std::endl;
	// const std::string				root
	// 	= take_location_root();
	// const std::string				reqPath(
	// 	http_req_take_url_path(req.at("url"), root)
	// );;
	// std::string						headers;
	// std::vector<char>				page;
	// std::string						filePath = "";

	// //TODO autoindex (CHECK that path refers to directory and not a regular file)
	// std::cout << "Path of the request : " << (reqPath.empty()? "EMPTY" : reqPath ) << std::endl;
	// if (reqPath.empty()) {
	// 	if (
	// 		(this->matching_directives.directives.find("autoindex") != this->matching_directives.directives.end())
	// 		&& (this->matching_directives.directives.at("autoindex") == "on" ))
	// 	{
	// 		std::string	path = req.at("url");
	// 		path_remove_leading_slash(path);
	// 		COUT_DEBUG_INSERTION("showing dir listing for " << root + path << std::endl)
	// 		std::string dir_listing_page = createHtmlPage(
	// 			getDirectoryContentList(root + path) //wip
	// 		);
	// 		page.insert(
	// 			page.begin(),
	// 			dir_listing_page.begin(),
	// 			dir_listing_page.end()
	// 		);
	// 		headers = getHeaders(200, "OK", filePath, page.size());
	// 		// throw (std::runtime_error("not yet implemented"));
	// 		// finire gestire ls
	// 	}
	// 	else
	// 		throw HttpError(404, this->matching_directives, root);
	// }
	// else {
	// 	COUT_DEBUG_INSERTION("serving page : " << root + reqPath << std::endl)
	// 									filePath = root + reqPath;
	// 	std::ifstream					docstream(filePath.c_str(), std::ios::binary);

	// 	if (false == docstream.is_open()) {
	// 		COUT_DEBUG_INSERTION("throwing page not found\n")
	// 		throw HttpError(404, matching_directives, take_location_root());
	// 	}
	// 	try {
	// 		page.insert(
	// 			page.begin(),
	// 			std::istreambuf_iterator<char>(docstream),
	// 			std::istreambuf_iterator<char>());
	// 	}
	// 	catch (const std::exception& e) {
	// 		COUT_DEBUG_INSERTION("throwing Internal Server Error\n")
	// 		throw HttpError(500, matching_directives, take_location_root());
	// 	}
	// 	headers = getHeaders(200, "OK", filePath, page.size());
	// }
	// response.insert(response.begin(), headers.begin(), headers.end());
	// response.insert(response.end(), page.begin(), page.end());
}
//*		non-member helper functions
