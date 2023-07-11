
// Field	Type	Required    Default		Possible Values	// Description		
// size		string	No			1M			1M or 1K	    // Sets the maximum body size for client requests.
                                							// Megabytes, M, and Kilobytes, K, are the accepted units.

# define	DEFAULT_CLIENT_MAX_BODY_SIZE	1000000			// 1M 1e+6 bytes
# define	LIMIT_CLIENT_MAX_BODY_SIZE		1000000000		// 1G 1e+9 bytes

# include <utility> // pair
# include <string>
# include <stdexcept>

// check if body size directive is correct
// config syntax 
// from 1 byte to 1e+9 bytes (1G) 
// 0 -> use default value
// default value -> 1M 1e+6 bytes
// limit value -> 1G 1e+9 bytes
// abbreviation accepted K and M 
// 1K 1e+3 bytes
// 1M 1e+6 bytes
// 1G 1e+9 bytes
bool	check_value_validity_body_size()
{
	if (current.directives.find("body_size") != current.directives.end())
	{
		std::string body_size_value = current.directives.at("body_size");

		// update body_size value if use of K or M abbreviation
		size_t pos = body_size_value.find_first_of("KM");
		if (pos != std::string::npos)
		{
			if (body_size_value.size() == pos + 1){
				std::string magnitude = body_size_value[pos] == 'K' ? "000" : "000000";
				body_size_value.erase(pos);
				body_size_value += magnitude;

				current.directives.at("body_size") = body_size_value;

			}
			else 
				throw (std::invalid_argument("Config::check_value_validity() : invalid value for body size directive."));
		}
		
		// check value validity using stoi (will throw if value isn't a number)
		try
		{
			int size = std::stoi(body_size_value);
			if (size > LIMIT_CLIENT_MAX_BODY_SIZE)
				throw (std::invalid_argument("Config::check_value_validity() : invalid value for body size directive (max 1G)."));
			if (size == 0)
				current.directives.at("body_size") = std::to_string(DEFAULT_CLIENT_MAX_BODY_SIZE);

		}
		catch (std::exception& e)
		{
			throw (std::invalid_argument("Config::check_value_validity() : invalid value for body size directive."));
		}			
	}
	else 
		current.directives.at("body_size") = std::to_string(DEFAULT_CLIENT_MAX_BODY_SIZE);

}

// check request against body size value, else throw 413 Content Too Large
	// value validity is checked in config
	// body_size always exists as its default value (1M) was set in value validity checking
bool	check_body_size()
{
	if (this->req.find("Content-Length") != this->req.end())
	{
		int req_body_size = std::stoi(this->req.at("Content-Length")); // stoi throws
		int max_body_size = std::stoi(this->matching_directives.directives.at("body_size"));

		if ( req_body_size > max_body_size )
			/*Content Too Large*/	throw HttpError(413, this->matching_directives, location_root);
	}
}