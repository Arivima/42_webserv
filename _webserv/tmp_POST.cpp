#include "include/Response.hpp"
#include <sys/socket.h>	//send
#include <fstream>		//open requested files
#include <sstream>		//stringstream
#include <cstring>		//memeset
#include <list>			//location matches sorting

void	Response::generatePOSTResponse( const std::string uri_path )
{
	const std::string				root = take_location_root(matching_directives, false);
	std::string						reqPath(uri_path);
	std::string						headers;
	std::string						tmp;
	std::vector<char>				body;
	std::string						filePath;

	std::cout << "uri_path : " << uri_path << std::endl;
	path_remove_leading_slash(reqPath);
	filePath = root + reqPath;
	
// delete the file/directory
	if (true == isDirectory("/", filePath, this->matching_directives)) {
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

