#pragma once

// defines
# define DEBUG 1

# define MAX_NB_OF_SERVER_BLOCKS     10
# define MAX_NB_OF_LOCATION_BLOCKS   10

# define BLOCK_NAME_ROOT        "root"
# define BLOCK_NAME_HTTP        "http"
# define BLOCK_NAME_SERVER      "server"
# define BLOCK_NAME_LOCATION    "location"

# define BLOCK_SEARCH_ROOT        "root {"
# define BLOCK_SEARCH_HTTP        "http {"
# define BLOCK_SEARCH_SERVER      "server {"
# define BLOCK_SEARCH_LOCATION    "location "

// enum
enum e_config_block_level {
    e_root_block = 0,
    e_http_block,
    e_server_block,
    e_location_block,
};

// prototypes
// std::string     type_get_name(size_t level);
int             type_get_level(std::string block_name);
size_t          type_get_max_nb(size_t level);
size_t          type_get_max_nb(size_t level);
size_t          type_get_min_nb(size_t level);

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

// struct
typedef struct s_conf_block {
    int                                 level;
    std::string                         block_name;
    std::vector<struct s_conf_block>	sub_blocks;
	std::map<std::string, std::string>	directives;

	s_conf_block() 
    : level(), block_name(), sub_blocks(), directives() {}

	s_conf_block(int level) 
    : level(level), block_name(type_get_name(level)), sub_blocks(), directives() {
        if (DEBUG) std::cout << "Level constructor" << std::endl;
        if (DEBUG) std::cout << "Build a new : " << this->block_name << " | " << this->level << std::endl;
    } // to initialize root

    s_conf_block(int lvl, std::map<std::string, std::string> dir) 
    : level(lvl), block_name(type_get_name(lvl)), sub_blocks(), directives(dir) {
        if (DEBUG) std::cout << "Directive constructor" << std::endl;
        if (DEBUG) std::cout << "Build from : " << (type_get_name(lvl - 1)) << " | " << lvl -1  << std::endl;
        if (DEBUG) std::cout << "Build a new : " << this->block_name << " | " << this->level << std::endl;
    }
}	t_conf_block;


/// getters


//WIP   // check find or compare
int type_get_level(std::string block_name){
    if (block_name.find(BLOCK_NAME_ROOT) != std::string::npos)
        return 0;
    if (block_name.find(BLOCK_NAME_HTTP) != std::string::npos)
        return 1;
    if (block_name.find(BLOCK_NAME_SERVER) != std::string::npos)
        return 2;
    if (block_name.find(BLOCK_NAME_LOCATION) != std::string::npos)
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

/// printing functions
/*brief*/   // prints contents of a map of string directives
void    print_directives(std::map<std::string, std::string>& directives){
    std::cout << "| Directives :" << std::endl;
    for (std::map<std::string, std::string>::iterator it = directives.begin(); it != directives.end(); it++)
        std::cout << "|  Key/Value |" << (*it).first << "|" << (*it).second << "|" << std::endl;
    std::cout << "--------------------------------------" << std::endl;
}
/*brief*/   // recursive call
void    print_block(t_conf_block& block, size_t level){
    std::cout << "--------------------------------------" << std::endl;
    std::cout << "| Printing print_level #" << block.level << " | " << block.block_name << std::endl;
    std::cout << "| Printing block_level #" << level << " |" << std::endl;
    print_directives(block.directives);
    for (std::vector<t_conf_block>::iterator it = block.sub_blocks.begin(); it != block.sub_blocks.end(); it++)
        print_block(*it, level + 1);
}
