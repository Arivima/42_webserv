#include "include/Response.hpp"
#include <sys/socket.h>	//send
#include <fstream>		//open requested files
#include <sstream>		//stringstream
#include <cstring>		//memeset
#include <list>			//location matches sorting




#include <cerrno>
#include <sys/stat.h>
#include <sys/types.h>

//_______________________ METHOD : POST _______________________
/**
 * @brief this function implements the POST method : creates a given file or directory and return a status of the operation
 * 
 * @param uri_path - the path of the file/dir to create
 * @exception throws HTTPError when given file or directory is already existent, path incorrect, or syscall functions fail
 * @return (void)
 */
void	Response::generatePOSTResponse( const std::string uri_path )
{
	const std::string				root = take_location_root(matching_directives, false);
	std::string						reqPath(uri_path);
	std::string						fullPath;
	std::string						newResourceName;

	std::cout << "POST uri_path : " << uri_path << std::endl;
	path_remove_leading_slash(reqPath);
	fullPath = root + reqPath;
	std::cout << "POST fullPath : " << fullPath << std::endl;

	// to update after - wip
	// if a filename is given in the request, UPDATE

	// check need to add extension

// checks if fullPath is pointing to a valid location (directory)
    struct stat						fileStat;
	bool							is_dir;

	errno = 0;
    if (stat(fullPath.c_str(), &fileStat) == 0) {
		is_dir = S_ISDIR(fileStat.st_mode);
		std::cout << MAGENTA << fullPath <<" : " << (is_dir? "is a directory" : "is not a directory") << RESET << std::endl;
		if (is_dir){
// create the new resource at the location
			std::cout << MAGENTA << "Trying to add a resource to DIRECTORY : " << fullPath << RESET << std::endl;

			// to update after - wip
			newResourceName = "POST_test";

			// check if file exists at the given location and updating the name if it is 
			while (stat((fullPath + "/" + newResourceName).c_str(), &fileStat) == 0) {
				newResourceName += "_cpy";
			}

			std::ofstream	stream_newResource((fullPath + "/" + newResourceName), std::ios::out);
			if (false == stream_newResource.is_open())
				throw_HttpError_debug("Response::generatePOSTResponse", "is_open()", 500, this->matching_directives);
			
			stream_newResource << std::string("Hello from Response::generatePOSTResponse()");
			stream_newResource << (req.find("body") != req.end() ? req.at("body") : std::string());
			
			if (stream_newResource.fail())
				throw_HttpError_debug("Response::generatePOSTResponse", "stream <<", 500, this->matching_directives);
			stream_newResource.close();

		}
		else {
			// to update after - wip
			std::cout << RED << "throwing 405 - Not allowed" << fullPath << RESET << std::endl;
			throw HttpError(405, matching_directives, root, "url should point to a directory (POST)");
		}
    }
	else
		throw_HttpError_errno_stat();


// update the response
	std::string						location_header;
	std::string						headers;
	std::string						tmp;
	std::vector<char>				body;
	std::string						domainName;
	std::string						newResourcePath;

	domainName		= std::string(server_IP) + ":" + matching_directives.directives.at("listen");
	newResourcePath	= "http://" + domainName + reqPath + "/" + newResourceName;
	location_header	= std::string("Location: " + newResourcePath);
	headers			= getHeaders(201, "OK", fullPath, body.size(), location_header);
	tmp				= "Resource " +  newResourcePath  + " at directory\"" +  reqPath  + "\" was successfully created\n";
	body.insert(body.begin(), tmp.begin(), tmp.end());
	this->response.insert(this->response.begin(), headers.begin(), headers.end());
	this->response.insert(this->response.end(), body.begin(), body.end());
}

// HTTP STATUS CODES FOR POST
// A successful response SHOULD be 
// 200 (OK)|		request was successful -> response body contains result or representation of requested resource
// 201 (Created)|	if the resource has been created on the origin server. response should a Location header with url of the created resource
// 204 (No Content)|request was successful -> no additional content to send back in the response body

// The action performed by the POST method might not result in a resource that can be 
// identified by a URI. In this case, either 200 (OK) or 204 (No Content) 
// is the appropriate response status, depending on whether or not the response 
// includes an entity that describes the result.
// If a resource has been created on the origin server, the response 
// SHOULD be 201 (Created) and contain an entity which describes the status of 
// the request and refers to the new resource, and a Location header (see section 14.30).

// failed request
// 400 Bad Request: The request could not be understood or was malformed. The server should include information in the response body or headers about the nature of the error.
// 403 Forbidden: The server understood the request, but the client does not have permission to access the requested resource.
// 404 Not Found: The requested resource could not be found on the server.
// 409 Conflict: The request could not be completed due to a conflict with the current state of the resource. This is often used for data validation errors or when trying to create a resource that already exists.
// 500 Internal Server Error: An unexpected error occurred on the server, indicating a problem with the server's configuration or processing of the request.

// STAT ERROR
// EACCES|	Search permission is denied for one of the directories in the path prefix of path. (See also path_resolution(7).)
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
