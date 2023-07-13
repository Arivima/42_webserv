/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinel <mmarinel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/13 17:08:49 by mmarinel          #+#    #+#             */
/*   Updated: 2023/07/13 17:11:28 by mmarinel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
# define UTILS_HPP

# include <string>
# include <ostream>
# include <vector>

# include "Types.hpp"
# include "Exceptions.hpp"

//*		general purpose utilities
struct			IsSpacePredicate {
	bool operator()(char c) const {
		return std::isspace(static_cast<unsigned char>(c));
	}
};


//	BLOCK HPP
t_config_block_level		next_conf_block_level(t_config_block_level lvl);
std::ostream&				operator<<(std::ostream& stream, const t_config_block_level& block);
void						print_block(t_conf_block& block, size_t level);
void						print_directives(std::map<std::string, std::string>& directives, size_t level);
int							block_get_level(std::string block_name);
std::string 				block_get_name(t_config_block_level level);
bool						mandatory_server_directives_present(const t_conf_block& current);
bool						same_server( const t_conf_block& server, const t_conf_block& virtual_serv2);
bool						same_host( const t_conf_block& virtual_serv1, const t_conf_block& virtual_serv2);
// std::string					take_location_root( const t_conf_block& matching_directives, bool file_upload );
bool						isCGI(
								const std::map<std::string, std::string>&	req,
								const t_conf_block&							matching_directives
							);
std::string					take_cgi_extension(
								const std::string& url,
								const std::map<std::string, std::string>& directives
							);
std::string					take_cgi_interpreter_path(
									const std::string& extension,
									const std::string& cgi_directive
							);

//	HTTP UTILS
bool						hasHttpHeaderDelimiter(std::vector<char>& line);
bool						isHttpHeaderDelimiter(std::vector<char>& line);
std::string					uri_remove_queryString(const std::string& uri);

//	UTILS EXCEPTIONS
void						check_file_accessibility(
								int						access_mode,
								const std::string&		fileRelPath,
								const std::string&		location_root,
								const t_conf_block&		matching_directives
							);
void						check_directory_deletable(
								const std::string&	dirRelPath,
								const std::string&	location_root,
								const t_conf_block&	matching_directives
							);
void						check_file_deletable(
								const std::string&		fileRelPath,
								const std::string&		location_root,
								const t_conf_block&		matching_directives
							);
HttpError					return_HttpError_errno_stat(
								const std::string& location_root,
								const t_conf_block& matching_directives
							);
void						throw_HttpError_debug(
								std::string function, std::string call,
								int httpStatusCode,
								const t_conf_block & matching_directives, const std::string& location_root
							);

//	UTILS HPP
std::string					strip_spaces(std::string& str);
void						strip_trailing_and_leading_spaces(std::string& str);
std::vector<std::string> 	split_str_to_vector(std::string s, const std::string& delimiter);
/**
 * @brief this function takes two white-spaced words and returns true iff they have words in common
 * 
 * @param str_haystack 
 * @param str_needle 
 * @return true 
 * @return false 
 */
bool						str_compare_words(const std::string& str_haystack, const std::string& str_needle);
void						path_remove_leading_slash(std::string& pathname);
bool						fileExists(
								const std::string& root,
								std::string path
							);
bool						isDirectory(const std::string root, std::string path);
std::string					getDirectoryContentList(const std::string directoryPath);
std::string					createHtmlPage(const std::string& title, const std::string& body);


#endif