/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinel <mmarinel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/18 10:41:29 by earendil          #+#    #+#             */
/*   Updated: 2023/06/30 12:40:11 by mmarinel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "include/Webserv.hpp"
#include <algorithm>
#include <cstring>
#include <sstream>//splitting : get_cgi_extension(), etc.

// BLOCK HPP
//*		TYPES CONSTRUCTORS
t_conf_block::	s_conf_block(
		t_config_block_level lvl,
		std::map<std::string, std::string> dir
	)
	: level(lvl), directives(dir), sub_blocks(), invalidated(false)
{
	COUT_DEBUG_INSERTION("Build a new : " << this->level << std::endl);
}

//*		TYPE UTILITIES
/// printing functions
/*brief*/   // prints contents of a map of string directives
void						print_directives(std::map<std::string, std::string>& directives, size_t level) {
	std::string	tabs(level, '\t');
	
	COUT_DEBUG_INSERTION(tabs << "| Directives :" << std::endl);
    for (std::map<std::string, std::string>::iterator it = directives.begin(); it != directives.end(); it++)
        COUT_DEBUG_INSERTION(tabs << "|  Key/Value |" << (*it).first << "|" << (*it).second << "|" << std::endl);
    COUT_DEBUG_INSERTION(tabs << "--------------------------------------" << std::endl);
}
/*brief*/   // recursive call
void						print_block(t_conf_block& block, size_t level) {
	std::string	tabs(level, '\t');
	
    COUT_DEBUG_INSERTION(tabs << "--------------------------------------" << std::endl)
    COUT_DEBUG_INSERTION(tabs << "| Printing print_level #" << block.level << std::endl)
    print_directives(block.directives, level);
    for (std::vector<t_conf_block>::iterator it = block.sub_blocks.begin(); it != block.sub_blocks.end(); it++)
        print_block(*it, level + 1);
}

std::ostream&				operator<<(std::ostream& stream, const t_config_block_level& block) {

	stream << block_get_name(block);

	return (stream);
}

t_config_block_level		next_conf_block_level(t_config_block_level lvl) {
	if (e_root_block == lvl)
		return (e_http_block);
	if (e_http_block == lvl)
		return (e_server_block);
	if (e_server_block == lvl)
		return (e_virtual_server_block);
	if (e_virtual_server_block == lvl)
		return (e_location_block);
	throw (
		std::runtime_error(
			"next_conf_block_level(): no level further than location")
		);
}

//WIP   // check find or compare
int							block_get_level(std::string block_name) {
	if ("root{" == strip_spaces(block_name))
		return e_root_block;
	if ("http{" == strip_spaces(block_name))
		return e_http_block;
	if ("server{" == strip_spaces(block_name))
		return e_server_block;
	if (0 == strncmp("location ", block_name.c_str(), strlen("location ")))
		return e_location_block;
	return -1;
}

std::string 				block_get_name(t_config_block_level level) {
	switch(level) {
		case e_root_block: return "root";
		case e_http_block: return "http";
		case e_server_block: return "server";
		case e_virtual_server_block: return "virtual server";
		case e_location_block: return "location";
	}
	return ("");
}

bool	mandatory_server_directives_present(const t_conf_block& current)
{
	static const char*							mandatory[] = {
		"listen", NULL//"host", "server_name", NULL
	};
	const std::map<std::string, std::string>&	directives
		= current.directives;
	size_t										i;

	i = 0;
	while (mandatory[i])
	{
		if (directives.end() == directives.find(mandatory[i]))
			return (false);
		i++;
	}
	return (true);
}

bool	same_server(
	const t_conf_block& server,
	const t_conf_block& virtual_serv2
)
{
	const std::map<std::string, std::string>&	server_dirs
		= server.sub_blocks[0].directives;
	const std::map<std::string, std::string>&	dirs2
		= virtual_serv2.directives;
		
	return (
		server_dirs.at("listen") == dirs2.at("listen")
		&&
		(
			//*	when host not set, server is listening on all
			//*	available local interfaces (IPs of the machine)
			server_dirs.end() == server_dirs.find("host") ||
			dirs2.end() == dirs2.find("host") ||
			server_dirs.at("host") == dirs2.at("host")

		)
	);
}

/**
 * @brief This function returns true iff there is a conflict between two virtual servers from the configuration.
 * Virtual servers are servers that share the same physical layer (ip and port).
 * The way to distinguish them is the use of server_names which define their virtual host.
 * There cannot be a virtual server that does not specify a server_name;
 * the server_name directive is only optional when there's only one server listening on an ip + port.
 * @param virtual_serv1 
 * @param virtual_serv2 
 * @return true 
 * @return false 
 */
bool	same_host(
	const t_conf_block& virtual_serv1,
	const t_conf_block& virtual_serv2
)
{
	const std::map<std::string, std::string>&	dirs1
		= virtual_serv1.directives;
	const std::map<std::string, std::string>&	dirs2
		= virtual_serv2.directives;
		
	return (
		dirs1.end() == dirs1.find("server_name") ||
		dirs2.end() == dirs2.find("server_name") ||
		str_compare_words(
			dirs1.at("server_name"), dirs2.at("server_name")
		)
	);
}

/**
 * @brief this function takes the root for the current location block.
 * Directive 'upload_path' for file upload requests,
 * directive 'root' for all other requests.
 * 
 * @param matching_directives 
 * @return std::string 
 */
std::string		take_location_root( const t_conf_block& matching_directives, bool file_upload )
{
	std::string											directive = file_upload ? "upload_path" : "root";
	std::string											root;
	std::map<std::string, std::string>::const_iterator	root_pos = matching_directives.directives.find(directive);

	if (matching_directives.directives.end() != root_pos) {
		root = matching_directives.directives.at(directive);
		path_remove_leading_slash(root);
		root += "/";
	}
	else
		root = "./";
	
	return (root);
}

bool	isCGI(
	const std::map<std::string, std::string>&	req,
	const t_conf_block&							matching_directives
)
{
	const std::string cgi_extension = take_cgi_extension(
			req.at("url"),
			matching_directives.directives
		);
		
	return (
		false == cgi_extension.empty()
	);
}

/**
 * @brief 
 * 
 * @param url 
 * @param directives 
 * @return std::string 
 */
std::string	take_cgi_extension(
	const std::string& url,
	const std::map<std::string, std::string>& directives
)
{
	size_t				cur_dot_pos;
	std::string			cur_extension;
	size_t				ext_found_at_pos;
	std::string			cgi_enable_directive;
	std::stringstream	cgiDirectiveStream;

	if (directives.end() == directives.find("cgi_enable"))
		return ("");
	cgi_enable_directive = directives.at("cgi_enable");
	cur_dot_pos = 0;
	while (true)
	{
		cur_dot_pos = cgi_enable_directive.find(".", cur_dot_pos);
		if (std::string::npos == cur_dot_pos)
			break ;
		
		cgiDirectiveStream.str(cgi_enable_directive.substr(cur_dot_pos));
		std::getline(cgiDirectiveStream, cur_extension, ' ');
		ext_found_at_pos = url.find(cur_extension);
		if (
			//*script at the end
			ext_found_at_pos + cur_extension.length() == url.length() ||
			//*additional url info
			'/' == url[ext_found_at_pos + cur_extension.length()]
		)
			return (cur_extension);
			
		cur_dot_pos += 1;
	}
	return "";
}




//*		GENERAL PURPOSE UTILITIES
std::string					strip_spaces(std::string& str) {
	std::string	stripped;

	stripped = str;
	stripped.erase(
		std::remove_if(stripped.begin(), stripped.end(), IsSpacePredicate()),
		stripped.end()
	);
	return (stripped);
}

void						strip_trailing_and_leading_spaces(std::string& str) {
	std::string::iterator			it;
	std::string::reverse_iterator	rit;

	//*		remove all trailing whitespaces up unitl first non-space char
	for (it = str.begin(); it != str.end(); ) {
		if (std::isspace(*it))
		{
			it = str.erase(it);
		}
		else {
			break ;
		}
	}
	//*		remove all leading whitespaces
	for (rit = str.rbegin(); rit != str.rend(); rit++) {
		if (std::isspace(*rit)) {
			it = (rit + 1).base();
			it = str.erase(it);
			rit = std::string::reverse_iterator(it);
		}
		else
			break;
	}
	// COUT_DEBUG_INSERTION("trimmed line (no leading and trailing whitespace): |" << str << std::endl)
}

/*brief*/	// split a string into a vector using delimiter space, without keeping whitespace
std::vector<std::string> split_str_to_vector( std::string s, const std::string& delimiter) {
	std::vector<std::string>	new_vector;
	std::string 				tmp;
	size_t 						pos;
	// COUT_DEBUG_INSERTION(MAGENTA "split_str_to_vector" RESET<< std::endl);
	// COUT_DEBUG_INSERTION("inital s : |" << s << "|" << std::endl);
	while ( s.size() > 0 ){
		strip_trailing_and_leading_spaces(s);
		if (s.empty())
			break ;
		pos = s.find(delimiter, 0);
		if (pos == std::string::npos)
			pos = s.size(); 
		tmp = s.substr(0, pos);
		s.erase(0, pos);
		new_vector.push_back(tmp);
	}
	// COUT_DEBUG_INSERTION(YELLOW"final vector : " RESET << std::endl);
	// if (new_vector.empty() == false){
	// 	for (std::vector<std::string>::iterator it = new_vector.begin(); it != new_vector.end(); it++)
	// 		COUT_DEBUG_INSERTION(YELLOW "|" << *it << "|" RESET << std::endl);
	// }
	return (new_vector);
}

/*brief*/	// compare two strings, return true if there is at least one common word
bool	str_compare_words(const std::string& str_haystack, const std::string& str_needle)
{
	std::vector<std::string> vector_haystack = split_str_to_vector(str_haystack, " ");
	std::vector<std::string> vector_needle = split_str_to_vector(str_needle, " ");
	COUT_DEBUG_INSERTION(MAGENTA"str_compare_words" RESET << std::endl);
	if ((vector_haystack.empty() == true) || (vector_needle.empty() == true)){
		COUT_DEBUG_INSERTION(MAGENTA"str_compare_words RETURN : negative match VOID VECTOR" RESET << std::endl);
		return (false);
	}

	for (
		std::vector<std::string>::iterator it_needle = vector_needle.begin();
		it_needle != vector_needle.end();
		it_needle++
	){
		for (
			std::vector<std::string>::iterator it_haystack = vector_haystack.begin();
			it_haystack != vector_haystack.end();
			it_haystack++
		){
			COUT_DEBUG_INSERTION("comparing |" << *it_needle << "| and |" << *it_haystack << "|" << std::endl);
			if ((*it_haystack).compare(*it_needle) == 0){
				COUT_DEBUG_INSERTION(MAGENTA"str_compare_words RETURN : positive match" RESET << std::endl);
				return (true);
			}
		}
	}
	COUT_DEBUG_INSERTION(MAGENTA"str_compare_words RETURN : negative match" RESET << std::endl);
	return (false);
}

void	path_remove_leading_slash(std::string& pathname)
{
	if (pathname.empty() == false && '/' == pathname[0])
		pathname = pathname.substr(1);
}

#include <cerrno>
#include <sys/stat.h>
bool			isDirectory(const std::string root, std::string path, const t_conf_block& matching_directives) {
    struct stat fileStat;

	path_remove_leading_slash(path);
	std::string dir_path = root + path;

    if (stat(dir_path.c_str(), &fileStat) == 0) {
		bool ret = S_ISDIR(fileStat.st_mode);
		COUT_DEBUG_INSERTION(MAGENTA << "isDirectory("<< dir_path <<"): " << (ret? "is a directory" : "is not a directory") << RESET << std::endl;);
		return ret;
        // return (S_ISDIR(fileStat.st_mode));
    }
	else{
		//TODO check errno for returning different HTTP error codes
		COUT_DEBUG_INSERTION(MAGENTA << "! isDirectory("<< dir_path <<"):" YELLOW " errno : " << errno << ":" << strerror(errno) << RESET << std::endl;);
		throw HttpError(404, matching_directives, root);
	}
    return (false);
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

std::string getDirectoryContentList(const std::string directoryPath)
{
	COUT_DEBUG_INSERTION(MAGENTA << "getDirectoryContentList("<< directoryPath <<"): " << RESET << std::endl;);
	std::cout << MAGENTA << "getDirectoryContentList : path ->"<< directoryPath << RESET << std::endl;
	std::string contentList;
    DIR* dir = opendir(directoryPath.c_str());
    if (dir){
        dirent* entry;
		errno = 0;
        while ((entry = readdir(dir)) != NULL){
			
			if (entry == NULL && errno != 0)								// errno, see above
				throw SystemCallException("readdir() : ");//*HttpError 500 server internal error

			std::string fileType;
			if (entry->d_type == DT_DIR)
				fileType = " | Directory ";
			else if (entry->d_type == DT_REG)
				fileType = " | Regular_file ";
			else
				fileType = " | Unkown_type ";

            std::string fileName = entry->d_name;
            if (fileName != "." && fileName != ".."){
                contentList += fileName + fileType + "\n";
            }
        }
        if (closedir(dir) != 0)
			throw SystemCallException("closedir()");//*HttpError 500 server internal error
    }
    else
		throw SystemCallException("opendir()");//*HttpError 500 server internal error
	COUT_DEBUG_INSERTION(YELLOW << contentList << RESET << std::endl;);
	std::cout << MAGENTA << contentList << RESET << std::endl;
    return contentList;
}

//TODO	mettere pathname directory nel titolo
std::string	createHtmlPage(const std::string& body)
{
	std::stringstream	pageStream;

	pageStream
	<< "<!DOCTYPE html>\
<html lang=\"en\">\
  <head>\
    <meta charset=\"UTF-8\" />\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />\
    <title>Directory Listing</title>\
  </head>\
  <body>\
    <h1>Directory Listing</h1>"
	<< body
	<< "</body>\
</html>\
";

	return (pageStream.str());
}

//!	NON PIU UTILE
// std::string	get_cgi_extension(
// 	const std::string& path,
// 	const std::map<std::string, std::string>& directives
// )
// {
// 	std::stringstream	cgiExtensionsStream;
// 	std::string			extension;
// 	size_t				semicolon_pos;
// 	bool				stop;

// 	if (directives.end() == directives.find("cgi_enable"))
// 		return ("");
// 	cgiExtensionsStream.str(directives.at("cgi_enable"));
// 	stop = false;
// 	while (false == stop)
// 	{
// 		std::getline(cgiExtensionsStream, extension, ' ');
// 		semicolon_pos = extension.find(";");
// 		if (std::string::npos != semicolon_pos) {
// 			stop = true;
// 			extension.erase(semicolon_pos);
// 		}
		
// 		if (std::string::npos != path.find(extension))
// 			return (extension);
// 	}
// 	return "";
// }


















































