// config examples
      // location /old-page {
      //     return 301 http://example.com/new-page;
      // }

      // location /old-page {
      //     return /new-page;
      // }




// update generate response
	if (matching_directives.directives.find("return") != matching_directives.directives.end()){
		handle_redirection(matching_directives.directives.at("return"));
	}
