#pragma once

typedef enum e_config_block_level {
    e_root_block = 0,
    e_http_block,
    e_server_block,
    e_location_block,
}

/*brief*/   // recursive call
template <typename Type, typename Subtype>
void    print_block(int level, Type block, Subtype sb){
    std::cout << "--------------------------------------" << std::endl;
    std::cout << "| Printing level #" << level << " | " << type_get_name(level) << std::endl;
    std::cout << "| Printing sublevel #" << sb.level<< std::endl;
    block.print_block_type();
    block.print_directives();
    for (std::vector<Subtype>::iterator it = block.sub_blocks.begin(); it != block.sub_blocks.end(); it++)
        (*it).print_block(level + 1);
}

// base block
typedef struct s_conf_block {
	std::map<std::string, std::string>	directives;
    std::vector<bool>                   sub_blocks;

    virtual void    print_type() { std::cout << "| Printing t_conf_block" << std::endl; }
    void            print_directives(){
        std::cout << "| Directives :" << std::endl;
        for (std::map<std::string, std::string>::iterator it = this->directives.begin(); it != this->directives.end(); it++)
            std::cout << "|  Key/Value |" << (*it).first << "|" << (*it).second << "|" << std::endl;
        std::cout << "--------------------------------------" << std::endl;
    }

}	t_conf_block;

// level blocks
typedef struct s_location_block : public t_conf_block {
    std::vector<t_conf_block>   sub_blocks;
    e_config_block_level level;
	s_location_block(const std::map<std::string, std::string>& dir) : sub_blocks(), level(e_location_block) {this->directives = dir;}

    virtual void    print_type() { std::cout << "| Printing t_location_block" << std::endl; }
    void            print_block(){ print_block<struct s_location_block, struct s_conf_block>(level, *this); }

}	t_location_block;

typedef struct s_server_block : public t_conf_block {
    std::vector<t_location_block> sub_blocks;
    e_config_block_level level;
	s_server_block(const std::map<std::string, std::string>& dir) : sub_blocks(), level(e_server_block) {this->directives = dir;}

    virtual void    print_type() { std::cout << "| Printing t_server_block" << std::endl; }
    void            print_block(){ print_block<struct s_server_block, struct t_location_block>(level, *this); }

}	t_server_block;

typedef struct s_http_block : public t_conf_block {
    std::vector<t_server_block> sub_blocks;
    e_config_block_level level;
	s_http_block(const std::map<std::string, std::string>& dir) : sub_blocks(), level(e_http_block) { this->directives = dir; }

    virtual void    print_type() { std::cout << "| Printing t_http_block" << std::endl; }
    void            print_block(){ print_block<struct s_http_block, struct t_server_block>(level, *this); }

}	t_http_block;

typedef struct s_conf_root_block : public t_conf_block {
    std::vector<t_http_block>   sub_blocks;
    e_config_block_level level;
	s_conf_root_block(const std::map<std::string, std::string>& dir) : sub_blocks(), level(e_root_block) { this->directives = dir; }

    virtual void    print_type() { std::cout << "| Printing t_conf_root_block" << std::endl; }
    void            print_block(){ print_block<struct s_conf_root_block, struct s_http_block>(level, *this); }

}	t_conf_root_block;





// typedef struct s_virtual_server_block : public t_conf_block {
//     std::vector<t_location_block>   sub_blocks;
// 	s_virtual_server_block(const std::map<std::string, std::string>& dir) : sub_blocks() {this->directives = dir;}
// }	t_virtual_server_block;

// typedef struct s_server_block : public t_conf_block {
//     std::vector<t_virtual_server_block> sub_blocks;
// 	s_server_block(const std::map<std::string, std::string>& dir) : sub_blocks() {this->directives = dir;}
// }	t_server_block;