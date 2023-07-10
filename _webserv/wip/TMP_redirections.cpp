// config examples
      // location /old-page {
      //     return 301 http://example.com/new-page;
      // }

      // location /old-page {
      //     return /new-page;
      // }




// to update
void	check_value_validity(std::string& key, std::string & value)
{
	if ("body_size" ==  key)
		check_value_validity_body_size(value);
	if ("return" ==  key)	// new
		check_value_validity_return(value);
}




// check if return directive is correct
// only one (optional) code, only one address 
// syntax -> return 307 http://example.com/new-page;
// syntax -> return http://example.com/new-page;

// 307	Temporary redirect		- Method and body not changed 
							//	- The Web page is temporarily unavailable for unforeseen reasons.
							//	- Better than 302 when non-GET operations are available on the site.
// 308	Permanent Redirect
							//	- Method and body not changed.
							//	- Reorganization of a website, with non-GET links/operations.
void	check_value_validity_return(std::string & value)
{
    std::istringstream					iss(value);
    std::pair<std::string, std::string> val;
    std::string							word;
	int									status_code;

	// Get the first word
	if (!(iss >> val.first))	
        throw (std::invalid_argument("Config::check_value_validity_return() : directive : return : empty input."));
	// Get the second word (optional)
    iss >> val.second;
	// Check if there are more than two words in the directive
	if (!val.second.empty()){
		iss >> word;
		if (!word.empty())	
			throw (std::invalid_argument("Config::check_value_validity_return() : directive : return : invalid config."));
	}

    // check code part of the directive
	try {
		status_code = std::stoi(val.first); // throws std::invalid_argument or std::out_of_range
		if ( status_code < 307 || status_code > 308 )
			throw (std::runtime_error("Config::check_value_validity_return() : directive : return : invalid config (redirection code allowed : 307, 308)."));
	}
	// if status_code is an int but not 307 nor 308
	catch (const std::runtime_error& e) { 
		throw (std::invalid_argument(e.what()));
	}
	// if status_code is not an int, it may be an address (as code is optional) // catching stoi exceptions
	catch (const std::exception& e) {
		// if no address provided
		if (val.second.empty())
			throw (std::invalid_argument("Config::check_value_validity_return() : directive : return : invalid config."));
		// -> manually correcting directive : updating default value for code (307)
		else					
			value = "307 " + word;
	}
}



// update generate response
	if (matching_directives.directives.find("return") != matching_directives.directives.end()){
		handle_redirection(matching_directives.directives.at("return"));
	}



// validity of configuration has been checked in the parsing
	// code can be 307 or 308
	// URL is validity is not checked -> responsibility of admin

// 307	Temporary redirect		- Method and body not changed 
							//	- The Web page is temporarily unavailable for unforeseen reasons.
							//	- Better than 302 when non-GET operations are available on the site.
// 308	Permanent Redirect
							//	- Method and body not changed.
							//	- Reorganization of a website, with non-GET links/operations.

void	handle_redirection(const std::string & value){
	// get the redirection information from the matching directives
    std::istringstream	iss(value);
	std::string			http_status_code;
	std::string			redirection_url;

    iss >> http_status_code;
    iss >> redirection_url;

	// update the response
	std::stringstream				headersStream;
	std::string						headers;

	headersStream
		<< "HTTP/1.1 " << http_status_code << " " << (http_status_code == 307 ? "Temporary redirect" : "Permanent Redirect") << "\r\n"
		<< std::string("Location: " + redirection_url) + "\r\n"
		<< "\r\n";
	
	headers = headersStream.str();
	this->response.insert(this->response.begin(), headers.begin(), headers.end());
}
