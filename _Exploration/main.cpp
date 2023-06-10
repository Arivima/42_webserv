// clear && g++ -Wall -Werror -Wextra main.cpp -o start && ./start

// throw ConfigFileException("");
    // check for bloc validity
        // wrong implementation of scope
        // server block missing
        // syntax error {} ; space
    // check for directive validity
        // directive missing, mandatory default values
        // directive unknown, syntax error
        // directive out of scope
        // directive duplicate
        // directive conflict
    // check for value validity
        // value invalid, syntax error
        // value invalid, file not found
        // value invalid, file unauthorized access

// mandatory directives
    // "listen", "location", "server_name", "index", "body_size", "error_page", 
    // "method", "root", "autoindex", "exec_cgi", "extension_cgi"
    
// assumptions
    //  block_name  string      max     min     type
    //  root        "root{"     1       1       s_conf_root_block
    //  http        "http{"     1       1       s_conf_http_block
    //  server      "server{"   +       1       s_conf_server_block
    //  location    "location{" +       0       s_conf_location_block

// includes
# include "include/Colors.hpp"
# include "Exceptions.hpp"

# include <iostream>
# include <stdexcept>
# include <string>
# include <unistd.h>
# include <fcntl.h>
# include <sstream>          // std::getline
# include <utility>          // std::pair

# include <vector>
# include <map>

// defines
# define RESET          "\033[0m"
# define BLACK          "\033[30m"
# define RED            "\033[31m"
# define GREEN          "\033[32m"
# define YELLOW         "\033[33m"
# define BLUE           "\033[34m"
# define MAGENTA        "\033[35m"
# define CYAN           "\033[36m"
# define WHITE          "\033[37m"

# define DEBUG              1 // set to 0/1 to hide/show debug info
# define DEFAULT_PATHNAME   "../_webserv/configuration_files/default.conf"

// struct
typedef struct s_test_block { //check then implement final structure
    std::vector<struct s_test_block>	sub_blocks;
	std::map<std::string, std::string>	directives;
	s_test_block() : sub_blocks(), directives() {}
	s_test_block(const std::map<std::string, std::string>& dir) : sub_blocks(), directives() { this->directives = dir; }
}	t_test_block;

// prototypes
std::string     read_config_file(std::string conf_pathname);
void            parse_configuration_file(std::string config_file);
void            parse_block(std::string& input, std::pair<int, int> current_range, t_test_block& block, int level);
void            parse_directives(t_test_block block, std::string input, std::pair<int, int> range);
int             find_end_of_block(std::string& input, int begin); 
void            print_directives(std::map<std::string, std::string>& directives);
void            print_block(t_test_block& block, int level);
void            print_root(t_test_block& root);
void            remove_comments(std::string * line);
std::string     type_get_name(int level);
int             type_get_max_nb(int level);
int             type_get_max_nb(int level);
int             type_get_min_nb(int level);
t_test_block&   type_get_obj(int level, std::map<std::string, std::string>& directives);

// main
int main (int ac, char**av){
    std::cout << "Welcome to mini-serv" << std::endl;
    try
    {
        if (ac > 2)
            throw std::invalid_argument("Wrong number of arguments.\n ./webserv [configuration_file]\n");

        std::string config_file = read_config_file(ac == 2 ? av[1] : DEFAULT_PATHNAME);
        parse_configuration_file(config_file);
        // initialization_configuration_file();
    }
	catch (std::exception& e)
    {
		std::cout << std::endl << RED "! Caught exception >>" << e.what() << RESET << std::endl;
    }
    return 0;
}

// functions
std::string    read_config_file(std::string conf_pathname){
    if (DEBUG) std::cout << YELLOW << "read_config_file()" << RESET << std::endl;

    // qui gestire meglio non so qual'e l'idea migliore
    int fd = open(conf_pathname.c_str(), O_RDONLY);
    if (fd == -1)
        throw std::invalid_argument("error open"); //!
    
    int buffer_size = 3000;
    char* buf = (char*) malloc(sizeof(char) * buffer_size);
    if (!buf){
        if (close(fd) == -1)
            throw std::invalid_argument("error malloc && close"); //!
        throw std::invalid_argument("error malloc"); //!
    }
    
    std::string str;
    int n;
    while ((n = read(fd, buf, buffer_size)) > 0){
        str.append(buf);
        buf = NULL;
    }
    if (DEBUG) std::cout << "str: " << std::endl << str << std::endl;
    if (n == -1){
        if (close(fd) == -1)
            throw std::invalid_argument("error read && close"); //!
        throw std::invalid_argument("error read"); //!
    }
    else if (n == 0){
        if (DEBUG) std::cout << MAGENTA << "EOF Finished reading configuration file" << RESET << std::endl;
    }
    return (str);
}


/*brief*/   // prints contents of a map of string directives
void    print_directives(std::map<std::string, std::string>& directives){
    std::cout << "| Directives :" << std::endl;
    for (std::map<std::string, std::string>::iterator it = directives.begin(); it != directives.end(); it++)
        std::cout << "|  Key :" << (*it).first << "| Value: " << (*it).second << std::endl;
    std::cout << "--------------------------------------" << std::endl;
}

/*brief*/   // recursive call
void    print_block(t_test_block& block, int level){
    std::cout << "--------------------------------------" << std::endl;
    std::cout << "| Printing level #" << level << std::endl;
    print_directives(block.directives);
    for (std::vector<struct s_test_block>::iterator it = block.sub_blocks.begin(); it != block.sub_blocks.end(); it++)
        print_block(*it, level + 1);
}

/*brief*/   // print all blocks from the root
void    print_root(t_test_block& root){
    std::cout << "--------------------------------------" << std::endl;
    std::cout << "| Printing configuration blocks" << std::endl;
    print_block(root, 0);
    std::cout << "--------------------------------------" << std::endl;
}

std::string type_get_name(int level){
    switch(level) {
        case 0: return "root";
        case 1: return "http";
        case 2: return "server";
        case 3: return "location";
    }
    throw std::invalid_argument("error get_type_name() : level invalid");
    return "error get_type_name()";
}

#define MAX_NB_OF_SERVER_BLOCKS     10
#define MAX_NB_OF_LOCATION_BLOCKS   10

int type_get_max_nb(int level){
    switch(level) {
        case 0: return 1;
        case 1: return 1;
        case 2: return MAX_NB_OF_SERVER_BLOCKS;
        case 3: return MAX_NB_OF_LOCATION_BLOCKS;
    }
    throw std::invalid_argument("error type_get_max_nb() : level invalid");
    return -1;
}

int type_get_min_nb(int level){
    switch(level) {
        case 0: return 1;
        case 1: return 1;
        case 2: return 1;
        case 3: return 0;
    }
    throw std::invalid_argument("error type_get_min_nb() : level invalid");
    return -1;
}

t_test_block& type_get_obj(int level, std::map<std::string, std::string>& directives){
    switch(level) {
        case 0: return t_test_block(directives);
        case 1: return t_test_block(directives);
        case 2: return t_test_block(directives);
        case 3: return t_test_block(directives);
    }
    throw std::invalid_argument("error type_get_min_nb() : level invalid");
    return t_test_block();
}

/*brief*/   // removes comment all comments = characters after a # and up until a \n
void    remove_comments(std::string * line){
    if (DEBUG) std::cout << YELLOW << "remove_comments()" << RESET << std::endl;
    for (size_t i = 0; (i = line->find("#", i)) != std::string::npos;){
        if (DEBUG) std::cout << "Found string to delete at pos : " << i << std::endl <<  RED << line->substr(i,  line->find("\n", i) - i) << RESET << std::endl;
        line->erase(i, line->find("\n", i) - i);
    }
    if (DEBUG) std::cout << * line << std::endl; 
}

std::pair<int, int> find_next_block_range(std::string& input, int begin_current, std::string next_block_name){
/*brief*/   // returns end of line of the first '}' character from position "begin" //OLD=> returns position of the first '}' character from position "begin"
            // input and begin validity are checked beforehand  
int    find_end_of_block(std::string& input, int begin){ 
    if (DEBUG) std::cout << YELLOW << "find_end_of_block()" << RESET << std::endl;

    int     bracket_flag    = 0;
    int     i               = input.find("{", begin);

    bracket_flag++;
    i++;
    for ( ; ( i < input.size() || bracket_flag == 0 ); i++ ){
        if (input.at(i) == "{")
            bracket_flag++;
        if (input.at(i) == "}")
            bracket_flag--;
    }
    if (bracket_flag)
        throw std::invalid_argument("error find_end_of_block() : end of block not found : missing }");
    // return i;
    // OR return end of line ??
    int ret = input.find("\n", i);
    if (ret == std::string::npos)
        return i;
    return ret;
}

/*brief*/   // returns the range (start, end) of the next block from position "begin_current", if there isn't returns (npos, npos)
            // return pos 0 of "xx{"
            //! works with a string
/*WIP*/     // leftover characters after '}' and before next '/n'
/*WIP*/     // levels and types
std::pair<int, int> find_next_block_range(std::string& input, int begin_current, std::string next_block_name){
    if (DEBUG) std::cout << YELLOW << "find_next_block_range()" << RESET << std::endl;

    int begin_new = input.find(next_block_name + "{", begin_current);
    if ( begin_new == std::string::npos ) {
        std::cout << "Testing find_end_of_block() : no next block" << std::endl;
        return (std::make_pair(std::string::npos, std::string::npos));
    }
    int end_new = find_end_of_block(input, begin_new); // throws if no '}'
    //! gerer les leftover characters before /n
    return (std::make_pair(begin_new, end_new));    
}

/*brief*/   // parse directives from "xx{" to next "xx{" or to end of block '}'
/*WIP*/     // check key validity
/*WIP*/     // check value validity
/*WIP*/     // ignore tabs and empty lines
void    parse_directives(t_test_block block, std::string input, std::pair<int, int> range){
    if (DEBUG) std::cout << YELLOW << "parse_directives()" << RESET << std::endl;
    std::string         line;
    std::istringstream  input_stream;

    input_stream.str(input.substr(range.first, range.second - range.first + 1));
    
    while (1){
        std::getline(input_stream, line);
        // ignore space tabs and empty lines
        if (input_stream.eof())
            break;
        else if (input_stream.fail())
            throw std::invalid_argument("error configuration file : parse_directives() : failbit getline()");
        else {
            if (line.find("{", 0) != std::string::npos || line.find("}", 0) != std::string::npos){
                std::cout << "parse_directives: NOT ADDING getline : " << line << std::endl;
                continue;
            }
            std::cout << "parse_directives: ADDING getline : " << line << std::endl;

            int pos_space = line.find(" ", 0);
            if (pos_space == std::string::npos)
                throw std::invalid_argument("error configuration file : directive syntax error : missing space");

            std::string key     = line.substr(line.front(), pos_space);
            std::string value   = line.substr(pos_space, line.back() - pos_space + 1);

            // check key validity
            // check value validity

            std::pair<std::map<std::string, std::string>::iterator, bool> ret;

            ret = block.directives.insert(std::make_pair(key, value));
            if (ret.second == false) // if key/value is already existing, value is updated
                (*(ret.first)).second = value;
        }
    }
}


/*brief*/   // recursive call to parse blocks
            // range indicates from "xx{" to next "xx{" or end of block '}'
            // range is always valid when entering this function
            // implement max and min number of blocks 
void    parse_block(std::string& input, std::pair<int, int> current_range, t_test_block& block, int level){
    if (DEBUG) std::cout << YELLOW << "parse_block()" << RESET << std::endl;
    if (DEBUG) std::cout << YELLOW << "| type_name" << type_get_name(level) << "| level" << level << " | Range " << current_range.first << " - " << current_range.second << RESET << std::endl;
    if (DEBUG) std::cout << YELLOW << "| first line " << input.substr(current_range.first, (int len = (current_range.second - current_range.first + 1)) > 20 ? 20 : len ) << RESET << std::endl;


    if (level + 1 <= 3){                                                                                            // root, http or server block
        std::pair<int, int> next_range = find_next_block_range(config_file, current_range.first, type_get_name(level + 1)); // (throws if not valid range)
        if ( next_range.first != std::string::npos ) {                                                              // if there is a next block                                       
            
            parse_directives(block, input, std::make_pair<int, int>(current_range.first, next_range.first));        // parse directives on same level until next block
            
            block.sub_blocks.pushback(type_get_obj(level + 1, block.directives));                                   // create a new sub-block from current block

            parse_block(block.sub_blocks.back(), input, next_range, level + 1);                                     // check & parse a new block on LOWER level

            parse_block(block, input, std::make_pair<int, int>(next_range.second, current_range.second), level);    // check & parse a new block on SAME level OR end of level directives
        }
    }
    if (level + 1 > 3 \                                                                                             // if location block
        ||  next_range.first != std::string::npos ) {                                                               // if no new blocks before '}'
        parse_directives(block, input, std::make_pair<int, int>(current_range.first, current_range.second));        // parse directives on same level until end of current block
    }
}

/*WIP*/     // add timeout
void    parse_configuration_file(std::string config_file){
    if (DEBUG) std::cout << YELLOW << "parse_configuration_file()" << RESET << std::endl;
    remove_comments(&config_file);

    t_test_block root;
    parse_block(config_file, std::make_pair<int, int>(config_file.front(), config_file.back()), root, 0);

    print_root(root);
}


// typedef struct s_conf_block {
// 	std::map<std::string, std::string>	directives;
// }	t_conf_block;

// typedef struct s_location_block : public t_conf_block {
// 	s_location_block(const std::map<std::string, std::string>& dir) {this->directives = dir;}
// }	t_location_block;

// typedef struct s_virtual_server_block : public t_conf_block {
// 	std::vector<t_location_block> locations;
// 	s_virtual_server_block(const std::map<std::string, std::string>& dir) : locations() {this->directives = dir;}
// }	t_virtual_server_block;

// typedef struct s_server_block : public t_conf_block {
// 	std::vector<t_virtual_server_block> virtual_servers;
// 	s_server_block(const std::map<std::string, std::string>& dir) : virtual_servers() {this->directives = dir;}
// }	t_server_block;

// typedef struct s_http_block : public t_conf_block {
// 	std::vector<struct s_server_block>	servers;
// 	s_http_block(const std::map<std::string, std::string>& dir) : servers() { this->directives = dir; }
// }	t_http_block;

// typedef struct s_conf_root_block : public t_conf_block {
// 	t_http_block	http;
// }	t_conf_root_block;



// ARCHIVE
// template < BlockType = s_conf_root_block >
// void    parse_new_block(std::string& input, int begin_current, int end_current, std::string next_block){
