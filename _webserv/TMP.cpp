
// Field	Type	Required    Default		Possible Values	// Description		
// size		string	No			1M			1M or 1K	    // Sets the maximum body size for client requests.
                                							// Megabytes, M, and Kilobytes, K, are the accepted units.
// check if body size directive is correct
# define DEFAULT_CLIENT_MAX_BODY_SIZE	1000000			// 1M 1e+6 bytes
# define LIMIT_CLIENT_MAX_BODY_SIZE		1000000000		// 1G 1e+9 bytes

# include <utility>
# include <string>

// config syntax : from  to 1000M (1G) // abbreviation accepted K and M // 0 (use defualt value)
bool	check_directives_validity()
{
	if (current.directives.find("body_size") != current.directives.end())
	{
		std::string body_size_value = current.directives.at("body_size");
		
		std::pair<int, char> max_size;

	}



}

// check request against body size value, else throw 413 Content Too Large
// default 1M
bool	check_body_size()
{
	if (this->matching_directives.directives.find("body") != this->req.end())
	// if (this->matching_directives.directives.find("Content-Lenght") != this->req.end())
	{
		int  max_size = 1 000 000; // 1M = 1e+6 bytes
		if (this->matching_directives.directives.find("body_size") != this->matching_directives.directives.end()){

			int size = 
			if (this->matching_directives.directives.at("body_size") )
		}
		else // default = 1M
		{

		}
	}


}