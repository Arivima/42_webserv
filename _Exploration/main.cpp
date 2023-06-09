# define RESET          "\033[0m"
# define BLACK          "\033[30m"
# define RED            "\033[31m"
# define GREEN          "\033[32m"
# define YELLOW         "\033[33m"
# define BLUE           "\033[34m"
# define MAGENTA        "\033[35m"
# define CYAN           "\033[36m"
# define WHITE          "\033[37m"

# include <iostream>
# include <stdexcept>
# include <string>
# include <unistd.h>
# include <fcntl.h>

# define DEBUG              1 // set to 0/1 to hide/show debug info
# define DEFAULT_PATHNAME   "../_webserv/configuration_files/default.conf"

// clear && g++ -Wall -Werror -Wextra main.cpp -o start && ./start

std::string read_config_file(std::string conf_pathname);
void        remove_comments(std::string * line);

int main (int ac, char**av){
    std::cout << "Welcome to mini-serv" << std::endl;
    try
    {
        if (ac > 2)
            throw std::invalid_argument("Wrong number of arguments.\n ./webserv [configuration_file]\n");

        std::string config_file = read_config_file(ac == 2 ? av[1] : DEFAULT_PATHNAME);
        remove_comments(&config_file);
        // parse_configuration_file(config_file);
        // initialization_configuration_file();
    }
	catch (std::exception& e)
    {
		std::cout << std::endl << RED "! Caught exception >>" << e.what() << RESET << std::endl;
    }
    return 0;
}

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

void    remove_comments(std::string * line){
    if (DEBUG) std::cout << YELLOW << "remove_comments()" << RESET << std::endl;
    for (size_t i = 0; (i = line->find("#", i)) != std::string::npos;){
        if (DEBUG) std::cout << "Found string to delete at pos : " << i << std::endl <<  RED << line->substr(i,  line->find("\n", i) - i) << RESET << std::endl;
        line->erase(i, line->find("\n", i) - i);
    }
    if (DEBUG) std::cout << * line << std::endl;    
}

