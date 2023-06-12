// clear && g++ -Wall -Werror -Wextra main.cpp -o start && ./start

// throw ConfigFileException("");
    // check for bloc validity
        //OK    // wrong implementation of scope
        //NOK    // server block missing
        //OK    // syntax error ; missing
        //NOK    // syntax error {} ; space
    // check for directive validity
        //OK    // directive missing, mandatory default values
        //OK    // directive unknown, syntax error
        //OK    // directive out of scope
        //OK    // directive duplicate
        //OK    // directive conflict
    // check for value validity
        //OK    // value invalid, syntax error
        //OK    // value invalid, file not found
        //OK    // value invalid, file unauthorized access

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
# include <iostream>
# include <stdexcept>
# include <string>
# include <unistd.h>
# include <fcntl.h>
# include <sstream>         // std::getline
# include <utility>         // std::pair
# include <limits>          // numeric limits

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

# define MAX_NB_OF_SERVER_BLOCKS     10
# define MAX_NB_OF_LOCATION_BLOCKS   10

# define BLOCK_NAME_ROOT        "root {"
# define BLOCK_NAME_HTTP        "http {"
# define BLOCK_NAME_SERVER      "server {"
# define BLOCK_NAME_LOCATION    "location "

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
void            parse_block(std::string& input, std::pair<size_t, size_t> current_range, t_test_block& block, int level);
void            parse_directives(t_test_block& block, std::string& input, std::pair<size_t, size_t> range);
size_t          find_end_of_block(std::string& input, size_t begin); 
std::string     get_whole_line(std::string& line, int i);
void            print_directives(std::map<std::string, std::string>& directives);
void            print_block(t_test_block& block, size_t level);
void            print_root(t_test_block& root);
void            remove_comments(std::string * line);
std::string     type_get_name(size_t level);
int             type_get_level(std::string block_name);
size_t          type_get_max_nb(size_t level);
size_t          type_get_max_nb(size_t level);
size_t          type_get_min_nb(size_t level);
t_test_block    type_get_obj(size_t level, std::map<std::string, std::string>& directives);
std::pair<bool, std::string> get_unknown_block_name(std::string& line, size_t i);

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
    
    size_t buffer_size = 3000;
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
    // if (DEBUG) std::cout << "str: " << std::endl << str << std::endl;
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

/// printing functions
/*brief*/   // prints contents of a map of string directives
void    print_directives(std::map<std::string, std::string>& directives){
    std::cout << "| Directives :" << std::endl;
    for (std::map<std::string, std::string>::iterator it = directives.begin(); it != directives.end(); it++)
        std::cout << "|  Key/Value |" << (*it).first << "|" << (*it).second << "|" << std::endl;
    std::cout << "--------------------------------------" << std::endl;
}

/*brief*/   // recursive call
void    print_block(t_test_block& block, size_t level){
    std::cout << "--------------------------------------" << std::endl;
    std::cout << "| Printing level #" << level << " | " << type_get_name(level) << std::endl;
    print_directives(block.directives);
    for (std::vector<struct s_test_block>::iterator it = block.sub_blocks.begin(); it != block.sub_blocks.end(); it++)
        print_block(*it, level + 1);
}

/*brief*/   // prsize_t all blocks from the root
void    print_root(t_test_block& root){
    std::cout << "--------------------------------------" << std::endl;
    std::cout << "| Printing configuration blocks" << std::endl;
    print_block(root, 0);
    std::cout << "--------------------------------------" << std::endl;
}

/// block management
std::string type_get_name(size_t level){
    switch(level) {
        case 0: return BLOCK_NAME_ROOT;
        case 1: return BLOCK_NAME_HTTP;
        case 2: return BLOCK_NAME_SERVER;
        case 3: return BLOCK_NAME_LOCATION;
    }
    throw std::invalid_argument("error get_type_name() : level invalid");
    return "error get_type_name()";
}

int type_get_level(std::string block_name){
    if (block_name.compare(BLOCK_NAME_ROOT) == 0)
        return 0;
    if (block_name.compare(BLOCK_NAME_HTTP) == 0)
        return 1;
    if (block_name.compare(BLOCK_NAME_SERVER) == 0)
        return 2;
    if (block_name.compare(BLOCK_NAME_LOCATION) == 0)
        return 3;
    return -1;
}

size_t type_get_max_nb(size_t level){
    switch(level) {
        case 0: return 1;
        case 1: return 1;
        case 2: return MAX_NB_OF_SERVER_BLOCKS;
        case 3: return MAX_NB_OF_LOCATION_BLOCKS;
    }
    throw std::invalid_argument("error type_get_max_nb() : level invalid");
    return -1;
}

size_t type_get_min_nb(size_t level){
    switch(level) {
        case 0: return 1;
        case 1: return 1;
        case 2: return 1;
        case 3: return 0;
    }
    throw std::invalid_argument("error type_get_min_nb() : level invalid");
    return -1;
}

t_test_block type_get_obj(size_t level, std::map<std::string, std::string>& directives){
    switch(level) {
        case 0: return t_test_block(directives);
        case 1: return t_test_block(directives);
        case 2: return t_test_block(directives);
        case 3: return t_test_block(directives);
    }
    throw std::invalid_argument("error type_get_min_nb() : level invalid");
    return t_test_block();
}


/*brief*/   // find any next block, and identifies the name 
            // does not work for location blocks
std::pair<bool, std::string> get_unknown_block_name(std::string& line, size_t i){
    size_t start, len;

    len = line.find("{", i);
    if (len == std::string::npos)
        return (std::make_pair(EXIT_FAILURE, "get_unknown_block_name : no next block found"));

    start = line.rfind("\n", len);
    if (start == std::string::npos)
        start = 0;
    else
        start++;
    len = len - start + 1;
    return (std::make_pair(EXIT_SUCCESS, line.substr(start, len)));
}

/*brief*/   // find any next block, and identifies the level, if not identified returns root level 0
            // does not work for location blocks
int get_unknown_block_level(std::string& line, size_t i){
    int level = 0;

    std::pair<bool, std::string> ret = get_unknown_block_name(line, i);
    if ( ret.first == EXIT_SUCCESS ){
        level = type_get_level(ret.second);
        if (level == -1)
            level = 0;
    }
    else{
        if (DEBUG) std::cout << RED << "get_unknown_block_level: error : " <<  ret.second << std::endl;
        level = 0;
    }
        
    return (level);
}

/*brief*/   // extracts the whole line where i is positioned
std::string    get_whole_line(std::string& line, int i){
    size_t start, len;
    start = line.rfind("\n", i);
    if (start != std::string::npos)
        start++;
    start = (start != std::string::npos ? start : i);
    len = line.find("\n", i);
    len = (len != std::string::npos ? len : i) - start + 1;
    return (line.substr(start, len));
}

/*brief*/   // removes comment all comments = characters after a # and up until a \n
void    remove_comments(std::string * line){
    if (DEBUG) std::cout << YELLOW << "remove_comments()" << RESET << std::endl;
    for (size_t i = 0; (i = line->find("#", i)) != std::string::npos;){
        if (DEBUG) std::cout << "Found string to delete at pos : " << i << std::endl <<  RED << get_whole_line(*line, i) << RESET << std::endl;
        line->erase(i, line->find("\n", i) - i);
    }
    // if (DEBUG) std::cout << * line << std::endl; 
}

/*brief*/   // returns end of line of the first '}' character from position "begin" //OLD=> returns position of the first '}' character from position "begin"
            // input and begin validity are checked beforehand  
size_t    find_end_of_block(std::string& input, size_t begin) {
    if (DEBUG) std::cout << YELLOW << "find_end_of_block()" << RESET << std::endl;
    // if (DEBUG) std::cout << YELLOW << "| first line: " << MAGENTA << get_whole_line(input, begin) << RESET << std::endl;

    int     bracket_flag    = 0;
    size_t  i               = input.find("{", begin);

    bracket_flag++;
    i++;
    for ( ; ( i <= input.size() && bracket_flag > 0 ); i++ ){
        // if (DEBUG) std::cout << MAGENTA << "| i: " << i << " | " << input.at(i) << "| bracket flag: " << bracket_flag << std::endl;
        if (input.at(i) == '{')
            bracket_flag++;
        else if (input.at(i) == '}')
            bracket_flag--;
    }
    // if (DEBUG) std::cout << MAGENTA << "| bracket flag: " << bracket_flag << std::endl;
    if (bracket_flag)
        throw std::invalid_argument("error find_end_of_block() : end of block not found : missing }");
    // if (DEBUG) std::cout << YELLOW << "| last line: |" << get_whole_line(input, i) << "|"<< RESET << std::endl;
    // return i;
    // OR return end of line ??
    size_t ret = input.find("\n", i);
    if (ret == std::string::npos)
        return i;
    return ret;
}

/*brief*/   // returns the range (start, end) of the next block from position "begin_current", if there isn't returns (npos, npos)
            // return pos 0 of "xx{"
            //! works with a string
/*WIP*/     // leftover characters after '}' and before next '/n'
/*WIP*/     // levels and types
std::pair<size_t, size_t> find_next_block_range(std::string& input, size_t begin_current, std::string next_block_name){
    if (DEBUG) std::cout << YELLOW << "find_next_block_range()" << RESET << std::endl;
    if (DEBUG) std::cout << YELLOW << next_block_name << RESET << std::endl;

    size_t begin_new = input.find(next_block_name, begin_current);

    // check wrong indentation
    size_t new_line = input.find("\n", begin_current);
    if (new_line ==std::string::npos)
        throw std::invalid_argument("error find_next_block_range()");
    size_t any_new = input.find("{", new_line);
    if ((any_new != std::string::npos) && ((begin_new == std::string::npos) || (any_new < begin_new))){
        std::pair<bool, std::string> any_name = get_unknown_block_name(input, any_new);
        throw std::invalid_argument("error find_next_block_range() : wrong indentation configuration file : " + any_name.second);
    }

    // no more block at lower level
    if ( begin_new == std::string::npos ) {
        // std::cout << "find_next_block_range() : no next block" << std::endl;
        return (std::make_pair(std::string::npos, std::string::npos));
    }

    // if there is a next lower block
    size_t end_new = find_end_of_block(input, begin_new); // throws if no '}'
    //! gerer les leftover characters before /n

    return (std::make_pair(begin_new, end_new));    
}

/*brief*/   // parse directives from "xx{" to next "xx{" or to end of block '}'
/*WIP*/     // check key validity
/*WIP*/     // check value validity
void    parse_directives(t_test_block& block, std::string& input, std::pair<size_t, size_t> range){
    int loop = 0; // debug to delete
    if (DEBUG) std::cout << RED << "parse_directives()" << RESET << std::endl;
    std::string         line;
    std::istringstream  input_stream;

    input_stream.str(input.substr(range.first, range.second - range.first + 1));
    
    while (1){
        std::getline(input_stream >> std::ws, line);
        // ignore space tabs and empty lines delimiter : ';' ???
        if (input_stream.eof()){
            // if (loop == 0)
            //     std::cout << RED << "No content" << RESET << std::endl;
            // else
            //     std::cout << RED << "Reached EOF" << RESET << std::endl;
            break;
        }
        else if (input_stream.fail())
            throw std::invalid_argument("error configuration file : parse_directives() : failbit getline()");
        else {
            if (((line.find("{", 0) != std::string::npos) && (line.find(BLOCK_NAME_LOCATION,0) == std::string::npos )) \
                    || line.find("}", 0) != std::string::npos){
                if (DEBUG) std::cout << "parse_directives: NOT ADDING getline : " << std::endl << line << std::endl;
                loop++;
                continue;
            }

            // if location block header
            if (( line.find("{", 0) != std::string::npos) && (line.find(BLOCK_NAME_LOCATION,0) != std::string::npos )){
                std::cout << line << std::endl;
                size_t erase_pos = ((line.find(" {", 0) != std::string::npos) ? line.find(" {", 0) : line.find("{", 0));
                line.erase(erase_pos);
            }
            else if (line.back() == ';') // any other directive line
                line.erase(line.size() - 1);
            else
                throw std::invalid_argument("error configuration file : directive syntax error : missing ;");

            // if (DEBUG) std::cout << GREEN << "parse_directives: ADDING getline :\t| " << RESET << line << std::endl;

            size_t pos_space = line.find(" ", 0);
            if (pos_space == std::string::npos)
                throw std::invalid_argument("error configuration file : directive syntax error : missing space");

            std::string key     = line.substr(0, pos_space);
            std::string value   = line.substr(pos_space + 1, line.size() - pos_space);

            // check key validity
            // check value validity

            // if (DEBUG) std::cout  << "parse_directives:\t" << GREEN << " key \t| " << key << " | value : " << value << RESET << std::endl;

            std::pair<std::map<std::string, std::string>::iterator, bool> ret;
            ret = block.directives.insert(std::make_pair(key, value));
            if (ret.second == false) // if key/value is already existing, value is updated
                (*(ret.first)).second = value;
        }
        loop++;
    }
}


/*brief*/   // recursive call to parse blocks
            // range indicates from "xx{" to next "xx{" or end of block '}'
            // range is always valid when entering this function
            // implement max and min number of blocks 
void    parse_block(std::string& input, std::pair<size_t, size_t> current_range, t_test_block& block, int level){
    if (DEBUG) std::cout << YELLOW << "parse_block()" << RESET << std::endl;
    if (DEBUG) std::cout << YELLOW << "| type_name : " << type_get_name(level) << " | level : " << level << " | Range " << current_range.first << " - " << current_range.second << RESET << std::endl;
    if (DEBUG) std::cout << YELLOW << "| first line : " MAGENTA << get_whole_line(input, current_range.first) << RESET << std::endl;


    if ( level < type_get_level(BLOCK_NAME_LOCATION) ){                                                                                            // root, http or server block
        std::pair<size_t, size_t> next_range = find_next_block_range( input, current_range.first, type_get_name(level + 1) ); // (throws if not valid range)
        if ( next_range.first != std::string::npos ) {                                                              // if there is a next block                                       
            std::cout << MAGENTA << "| Found next block" << RESET << std::endl;
            parse_directives(block, input, std::make_pair( current_range.first, next_range.first ) );        // parse directives on same level until next block
            
            block.sub_blocks.push_back( type_get_obj( level + 1, block.directives ) );                                   // create a new sub-block from current block

            parse_block( input, next_range, block.sub_blocks.back(), level + 1 );                                     // check & parse a new block on LOWER level

            if (DEBUG) std::cout << MAGENTA << "| continuing parsing block : " << type_get_name(level) << RESET << std::endl;
            parse_block( input, std::make_pair( next_range.second, current_range.second ), block, level );    // check & parse a new block on SAME level OR end of level directives
        }
        else {                                                                                                    // if no new blocks before '}'
            std::cout << MAGENTA << "| NO next block" << RESET << std::endl;
            parse_directives( block, input, std::make_pair( current_range.first, current_range.second ) );        // parse directives on same level until end of current block
        }
    }
    if ( type_get_name(level) == BLOCK_NAME_LOCATION ) {                                                                                       // if location block OR 
        if (DEBUG) std::cout << MAGENTA << "| Reached max level : " << type_get_name(level) << " | remaining content in block is parsed as directives " << RESET << std::endl;
        parse_directives( block, input, std::make_pair( current_range.first, current_range.second ) );        // parse directives on same level until end of current block
    }
}

/*brief*/   // parse configuration file into configuration structures, checks validity of input
//TODO      // add the real structures
//TODO      // add all validity checks
//TODO      // redo a flow run to catch all loose error/exception/cases threads
//TODO      // thorough testing
/*WIP*/     // add timeout
void    parse_configuration_file(std::string config_file){

    remove_comments(&config_file);

    if (DEBUG) std::cout << YELLOW << "parse_configuration_file()" << RESET << std::endl;
    size_t          start    = 0;
    size_t          end      = config_file.size() - 1 ;
    t_test_block    root_block;
    size_t          level    = get_unknown_block_level(config_file, 0);

    parse_block(config_file, std::make_pair( start, end ), root_block, level );

    print_root(root_block);
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
// void    parse_new_block(std::string& input, size_t begin_current, size_t end_current, std::string next_block){
