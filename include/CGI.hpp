/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*   By: team_PiouPiou                                +:+ +:+         +:+     */
/*       avilla-m <avilla-m@student.42.fr>          +#+  +:+       +#+        */
/*       mmarinel <mmarinel@student.42.fr>        +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*                                                    ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HPP
# define CGI_HPP

# include	<map>
# include	<vector>
# include	<string>

# include	<unistd.h>		// pipe
# include	<sys/wait.h>	// wait

# include	"Webserv.hpp"
# include	"Request.hpp"

# define	CGI_ENV_SIZE	31
# define	CGI_OUTFILE		".cgi_output"

class CGI {
//*		Private member attributes ___________________________________
private:
	const int									sock_fd;
	char*										cgi_env[CGI_ENV_SIZE + 1];
	std::vector<char>							response;
	Request*									request;
	const std::map<std::string, std::string>&	req;
	const t_conf_block&							matching_directives;
	int											sendPayload_pipe[2];
	int											fd_out;	//response file descriptor
	int											backupStdout;
	pid_t										pid;
	bool										chunked;
	long										max_body_size;

//*		Public member functions _____________________________________
public:
	//*	main Constructors and destructors
	CGI(
		int											sock_fd,
		const std::string &							client_IP,
		const std::string &							server_IP,
		Request*									request,
		bool										chunked,
		const t_conf_block &						matching_directives,
		const std::string&							location_root,
		const std::string &							cgi_extension,
		const std::string &							interpreter_path
	);
	~CGI();
	//*	main functionalities
	void				launch();
	void				CGINextChunk( void );
	std::vector<char>	getResponse();
	std::string			get_env_value(const std::string & key);

//*		Private member functions ____________________________________
private:
	void				init_env_paths(const std::string & root, const std::string & cgi_extension, const std::string & interpreter_path);
	void				init_env(const t_conf_block& matching_directives, const std::string& client_IP, const std::string& server_IP);
	//* Unused canonical form
	CGI();
	CGI(const CGI & c);
	CGI&				operator=(const CGI & c);
	//*	helper functions
	void				print_arr(char ** arr, const std::string & title);
};
#endif
