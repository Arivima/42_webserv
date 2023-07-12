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

// example				http://localhost:80/fragola/lemon.txt
// example				POST /fragola/lemon.txt
// uri_path		client	/fragola/lemon.txt
// reqPath				fragola/lemon.txt

// root			config	/var/www/html
// upload_path	config	uploads
// Dir					fragola
// newFileName	client	lemon.txt

// fullFilePath			/var/www/html/uploads/fragola/lemon.txt
// fullDirPath			root + "/" + upload_path + "/" + dir
// fullFilePath			root + "/" + upload_path + "/" + dir + "/" + newFileName

//TODO
//TODO	1. line 49 : check if upload_path must be appended to the root
//TODO	2. line 53 : move (and maybe correct) check in generateResponse
//TODO	3. line 69 : check exact behavior
//TODO	4. line 103 : check max path length
//TODO	5. line 110 : check open_mode
//TODO	6. line 139 : check if location points to an absolute or relative path
//TODO
void	Response::generatePOSTResponse( const std::string uri_path )
{
	const std::string				root = take_location_root(matching_directives, false);
	std::string						upload_path = take_location_root(matching_directives, true);
	std::string						reqPath(uri_path);
	std::string						dir;
	std::string						newFileName;
	std::string						fullDirPath;
	std::string						fullFilePath;

	COUT_DEBUG_INSERTION("POST uri_path : " << uri_path << std::endl);
	//! to move to generate response
	if (reqPath.find("/..") != std::string::npos)
		/*Bad req*/	throw HttpError(400, matching_directives, root);  	

	path_remove_leading_slash(upload_path);
	path_remove_leading_slash(reqPath);

	// check if existing filename and splits reqPath into dir and newFileName
	size_t pos = reqPath.rfind("/");
	if (pos != std::string::npos){
		dir = reqPath.substr(0, pos);
		newFileName = reqPath.substr(pos + 1);
	}
	else { // if no filename given
		dir = reqPath;
		//! check behavior ? throw ?
		newFileName = "POST_test";
	}

	//! check upload_path is indeed correct
	fullDirPath		= root + "/" + upload_path + "/" + dir;
	fullFilePath	= root + "/" + upload_path + "/" + dir + "/" + newFileName;

	COUT_DEBUG_INSERTION("POST reqPath : "	 	<< reqPath << std::endl);
	COUT_DEBUG_INSERTION("POST root : "	 		<< root << std::endl);
	COUT_DEBUG_INSERTION("POST upload_path : "	<< upload_path << std::endl);
	COUT_DEBUG_INSERTION("POST dir : "	 		<< dir << std::endl);
	COUT_DEBUG_INSERTION("POST newFileName : "	<< newFileName << std::endl);
	COUT_DEBUG_INSERTION("POST fullDirPath : "	<< fullDirPath << std::endl);
	COUT_DEBUG_INSERTION("POST fullFilePath : "	<< fullFilePath << std::endl);

// checks if fullDirPath is pointing to a valid location (directory)
    struct stat						fileStat;
	bool							is_dir;

	errno = 0;
    if (stat(fullDirPath.c_str(), &fileStat) == 0) {
		is_dir = S_ISDIR(fileStat.st_mode);
		COUT_DEBUG_INSERTION(MAGENTA << fullDirPath <<" : " << (is_dir? "is a directory" : "is not a directory") << RESET << std::endl);
		if (is_dir){
// create the new resource at the location
			COUT_DEBUG_INSERTION(MAGENTA << "Trying to add a resource to DIRECTORY : " << fullDirPath << RESET << std::endl);

			// check if file exists at the given location and updating the name if it does
			//TODO Replace with fileExists ?
			while (stat(fullFilePath.c_str(), &fileStat) == 0) {
				std::string extension = take_cgi_extension(newFileName, this->matching_directives);
				if (extension.empty() == false)
					newFileName	= newFileName.substr(0, newFileName.find(extension));
				newFileName += "_cpy" + extension;
				if (newFileName.size() >= 255)
					/*Server Err*/	throw HttpError(500, this->matching_directives, root);
			}
			fullFilePath	= root + "/" + upload_path + "/" + dir + "/" + newFileName;
			
			// writes body to new file
			std::ofstream	stream_newFile(fullFilePath/*, std::ios::out*/);
			if (false == stream_newFile.is_open())
				/*Server Err*/	throw HttpError(500, this->matching_directives, root);
			
			stream_newFile << std::string("Hello from Response::generatePOSTResponse()");
			stream_newFile << (this->req.find("body") != this->req.end() ? this->req.at("body") : std::string());
			
			if (stream_newFile.fail())
				/*Server Err*/	throw HttpError(500, this->matching_directives, root);
			stream_newFile.close();
		}
		else
			/*Not allowed*/	throw HttpError(405, this->matching_directives, root, "url should point to a directory (POST)");
    }
	else
		this->throw_HttpError_errno_stat();


// update the response
	std::string						location_header;
	std::string						headers;
	std::string						tmp;
	std::vector<char>				body;
	std::string						domainName;
	std::string						newResourcePath;

	domainName		= std::string(server_IP) + ":" + matching_directives.directives.at("listen");
	newResourcePath	= "http://" + domainName + "/" + dir + "/" + newFileName;

	location_header	= std::string("Location: " + newResourcePath);
	headers			= getHeaders(201, "OK", (dir + "/" + newFileName), body.size(), location_header);
	this->response.insert(this->response.begin(), headers.begin(), headers.end());

	tmp				= "Resource " +  newResourcePath  + " at directory\"" +  reqPath  + "\" was successfully created\n";
	body.insert(body.begin(), tmp.begin(), tmp.end());
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
