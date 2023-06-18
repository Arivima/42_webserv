/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/18 10:41:29 by earendil          #+#    #+#             */
/*   Updated: 2023/06/18 15:37:15 by earendil         ###   ########.fr       */
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
