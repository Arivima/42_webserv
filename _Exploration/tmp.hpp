
#include <vector>
#include <map>

struct s_http_block;

typedef struct s_main_block {
	std::map<std::string, std::string>	directives;

	struct s_http_block*	http;
}	t_main_block;

typedef struct s_http_block : public t_main_block {

	std::vector<struct s_server_block*> servers;

}	t_http_block;


typedef struct s_server_block : public t_http_block {

	std::vector<struct s_location_block*> servers;

	s_server_block(const std::map<std::string, std::string>& directives) {
		this->directives = directives;
	}
}	t_server_block;

typedef struct s_location_block : public t_server_block {

	s_location_block(const std::map<std::string, std::string>& directives) {
		this->directives = directives;
	}
}	t_location_block;
