#ifndef CGI_HPP
# define CGI_HPP

# include <map>
# include <string>
# include <sstream>
// # include <utility>

# include "Webserv.hpp"

class CGI {
private:
    CGI();
    CGI(const CGI & cpy);
    CGI& operator=(const CGI & cpy);
    std::vector<char>       response;

public:
    CGI(const std::map<std::string, std::string> req, const t_conf_block& matching_directives);
    ~CGI();

    launch(); 
    std::vector<char> getResponse()



}

#endif