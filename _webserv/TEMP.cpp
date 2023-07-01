#include "include/Response.hpp"
#include <sys/socket.h>	//send
#include <fstream>		//open requested files
#include <sstream>		//stringstream
#include <cstring>		//memeset
#include <list>			//location matches sorting

#include <unistd.h>		// access
#include <cerrno>		// errno
#include <stdio.h>		// remove

void	Response::generateDELETEResponse( const std::string uri_path )
{
	const std::string				upload_path = take_location_root(matching_directives, true);
	std::string						reqPath(req.at("url"));
	std::string						headers;
	std::vector<char>				page;
	std::string						filePath = "";

	path_remove_leading_slash(reqPath);
	filePath = upload_path + reqPath;
	
	COUT_DEBUG_INSERTION("trying to create file : " << upload_path + reqPath << std::endl)
	std::ifstream					newFileStream(filePath.c_str(), std::ios_binary);

// is the method allowed in this folder
// 405 Method Not Allowed - The request method is known by the server but is not supported by the target resource. 
    if (this->matching_directives.directives.find("method") != this->matching_directives.directives.end() \
        && this->matching_directives.directives.at("method").find("DELETE") == std::string::npos)
        throw HttpError(405, matching_directives, take_location_root(matching_directives, false));

// Is the path a directory
	// recursive function
	// execute deleteFile() on all directory entries and subentries // if empty dir -> remove

// deleteFile() 
// does the resource exists
// 404 Not Found - The server cannot find the requested resource. In the browser, this means the URL is not recognized. 
    if (access(filePath.c_str(), F_OK) == -1)
        throw HttpError(404, matching_directives, take_location_root(matching_directives, false));

// do I have the right permission ? 
// 401 Unauthorized - Although the HTTP standard specifies "unauthorized", semantically this response means "unauthenticated". 
	// That is, the client must authenticate itself to get the requested response.
// 403 Forbidden - The client does not have access rights to the content; that is, it is unauthorized, so the server is refusing to give the requested resource. 
	// Unlike 401 Unauthorized, the client's identity is known to the server.
// 409 Conflict: The deletion could not be completed due to a conflict with the current state of the resource. 
	// ETXTBSY		Write access was requested to an executable which is being executed.
    if (access(filePath.c_str(), W_OK) == -1){
		if (errno == ETXTBSY)
			throw HttpError(409, matching_directives, take_location_root(matching_directives, false));
		else
			throw HttpError(401, matching_directives, take_location_root(matching_directives, false));

	}

	// unlink

// 410 Gone: The resource has been permanently deleted and is no longer available. // works with logs probably
// 500 Internal Server Error: An unexpected error occurred on the server while processing the deletion request.
// A successful response SHOULD be 200 (OK) if the response includes an entity describing the status, 202 (Accepted) if the action has not yet been enacted, or 204 (No Content) if the action has been enacted but the response does not include an entity.



// delete the resource
// do we need to implement 2002 and 200 separately
// update the response


}



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

void	Response::generatePOSTResponse( const std::string uri_path )
{
	const std::string				upload_path = take_location_root(matching_directives, true);
	std::string						reqPath(req.at("url"));
	std::string						headers;
	std::vector<char>				page;
	std::string						filePath = "";

	path_remove_leading_slash(reqPath);
	filePath = upload_path + reqPath;
	
	COUT_DEBUG_INSERTION("trying to create file : " << upload_path + reqPath << std::endl)
	std::ifstream					newFileStream(filePath.c_str(), std::ios_binary);

}
