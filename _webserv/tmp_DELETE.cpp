#include "include/Response.hpp"
#include <sys/socket.h>	//send
#include <fstream>		//open requested files
#include <sstream>		//stringstream
#include <cstring>		//memeset
#include <list>			//location matches sorting

#include <unistd.h>		// access, unlink
#include <cerrno>		// errno
#include <stdio.h>		// remove


void Response::deleteFile( const std::string filePath ){
// does the resource exists
	errno = 0;
    if (access(filePath.c_str(), F_OK) == -1)
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


#include <iostream>
#include <string>
#include <dirent.h>
#include <cerrno>

// readdir()
//		  RETURN VALUE 
//        On success, readdir() returns a pointer to a dirent structure.
//        If the end of the directory stream is reached, NULL is returned
//        and errno is not changed.  If an error occurs, NULL is returned
//        and errno is set to indicate the error.  To distinguish end of
//        stream from an error, set errno to zero before calling readdir()
//        and then check the value of errno if NULL is returned.

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

void	Response::generateDELETEResponse( const std::string uri_path )
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

