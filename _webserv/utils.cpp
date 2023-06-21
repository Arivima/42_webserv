/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/18 10:41:29 by earendil          #+#    #+#             */
/*   Updated: 2023/06/21 18:42:13 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "include/webserv.hpp"
#include <algorithm>
#include <cstring>

//*		TYPES CONSTRUCTORS
t_conf_block::	s_conf_block(
		t_config_block_level lvl,
		std::map<std::string, std::string> dir
	)
	: level(lvl), block_name(block_get_name(lvl)), directives(dir), sub_blocks()
{
	// if (e_root_block == lvl)
	// 	this->directives.erase(this->directives.begin(), this->directives.end());
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
    COUT_DEBUG_INSERTION(tabs << "| Printing print_level #" << block.level << " | " << block.block_name << std::endl)
    COUT_DEBUG_INSERTION(tabs << "| Printing block_level #" << level << " |" << std::endl)
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

#include <algorithm>

/*brief*/	// split a string into a vector using delimiter space, without keeping whitespace
std::vector<std::string> split_str_to_vector( std::string s, const std::string& delimiter){
	std::vector<std::string>	new_vector;
	std::string 				tmp;
	size_t 						pos;
	COUT_DEBUG_INSERTION(MAGENTA "split_str_to_vector" RESET<< std::endl);
	COUT_DEBUG_INSERTION("inital s : |" << s << "|" << std::endl);
	while ( s.size() > 0 ){
		COUT_DEBUG_INSERTION("start loop" << std::endl);
		COUT_DEBUG_INSERTION("str size |" << s.size() << "|" << std::endl);
		strip_trailing_and_leading_spaces(s);
		if (s.empty())
			break ;
		COUT_DEBUG_INSERTION("before s : |" << s << "|" << std::endl);
		pos = s.find(delimiter, 0);
		if (pos == std::string::npos)
			pos = s.size(); 
		tmp = s.substr(0, pos);
		s.erase(0, pos);
		COUT_DEBUG_INSERTION("after s : |" << s << "|" << std::endl);
		COUT_DEBUG_INSERTION("tmp : |" << tmp << "|" << std::endl);
		new_vector.push_back(tmp);
	}
	COUT_DEBUG_INSERTION(YELLOW"final vector : " RESET << std::endl);
	if (new_vector.empty() == false){
		for (std::vector<std::string>::iterator it = new_vector.begin(); it != new_vector.end(); it++)
			COUT_DEBUG_INSERTION(YELLOW "|" << *it << "|" RESET << std::endl);
	}
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

//! alternative with stream

    // std::istringstream iss(inputString);
    // std::vector<std::string> words;
    // std::string word;
    // while (iss >> word)
    // {
    //     words.push_back(word);
    // }

// bool	str_compare_words(const std::string& s1, const std::string& s2)
// {
// 	std::stringstream	stream;
// 	stream.str(this->s1);
// 	while (stream.good()) {
//         std::getline(stream /*>> std::ws*/, line, ' ');
// 		strip_trailing_and_leading_spaces(line);
// 		COUT_DEBUG_INSERTION("line read: |" << line << std::endl);
		
// 		if (std::string::npos != s2.find(line)) {
// 			// can return true if line is a substring of a word in s2
// 			return (true);
// 		}
// 	}
// 	if (stream.bad())
// 		throw (std::runtime_error("Utils str_compare_words() : IO corrupted"));

// 	return (false);
// }


#include <set>
/*brief*/	// checks the directive against the dictionary
			// return 0 if not valid, 1 if valid
bool 	is_valid_directive(const std::string & s){


    const char* strings[] = {
	"listen", "location", "server_name", "index", "body_size", 
	"error_page", "method", "root", "return", "autoindex", "exec_cgi", "extension_cgi"
	};

    std::set<std::string> dictionnary_directives(strings, strings + sizeof(strings) / sizeof(strings[0]));



	// static std::set<const std::string> dictionnary_directives({
	// "listen", "location", "server_name", "index", "body_size", 
	// "error_page", "method", "root", "return", "autoindex", "exec_cgi", "extension_cgi"
	// });
	
	return (dictionnary_directives.count(s));
}