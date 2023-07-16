/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Worker.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinel <mmarinel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/06 21:15:29 by earendil          #+#    #+#             */
/*   Updated: 2023/07/16 18:09:13 by mmarinel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Worker.hpp"

//*		main constructors and destructor

Worker::Worker(const t_conf_block& root_block) : servers(), edata()
{
	const t_conf_block&							http_block = root_block.sub_blocks[0];
	std::vector<t_conf_block>::const_iterator	server;
	std::string									debug_server_name;

	for (
		server = http_block.sub_blocks.begin();
		server != http_block.sub_blocks.end();
		server++
	)
	{
		try {
			this->_server_init(*server);
		}
		catch (const std::exception& e) {
			//*	We should never be able to get here if all conflicts are handled in config parsing (class Config)
			//*	catch is entered iff a socket syscall (e.g.: bind throws an error)
			
			if ((*server).directives.end() != (*server).directives.find("server_name"))
				debug_server_name = (*server).directives.at("server_name");
			else
				debug_server_name = "N/A";
			std::cout << BOLDRED
				<< "server creation failed " << std::endl
				<< "listen : " << (*server).sub_blocks[0].directives.at("listen") << std::endl
				<< "server_name : " << debug_server_name << std::endl
				<< "reason : " << e.what() << std::endl
				<< RESET;

			this->servers.pop_back();
		}
	}
	if (0 == this->servers.size())
		throw (std::runtime_error("Worker::Worker() : no server could be started"));
	else {
		COUT_DEBUG_INSERTION(
			this->servers.size() << " servers started" << std::endl;
		);
		_init_io_multiplexing();
	}
}

Worker::~Worker() {
	//*	VectorServ is a vector of statically allocated t_server objects.
	//*	Each t_server object has a vector of dynamically allocated open connections
	for (
		VectorServ::iterator serv_it = this->servers.begin();
		serv_it != this->servers.end();
		serv_it++
	)
	{
		ConnectionSocket*	connection;
		
		for (
			VectorCli::iterator conn_it = (*serv_it).open_connections.begin();
			conn_it != (*serv_it).open_connections.end();
			/*...no increment*/
		)
		{
			connection = (*conn_it);
			conn_it = (*serv_it).open_connections.erase(conn_it);
			delete (connection) ;
		}
		(*serv_it).open_connections.clear();
		std::cout << BOLDRED<< "Closing server: " << (*serv_it).server_fd << RESET << std::endl;
		close((*serv_it).server_fd);
	}
	this->servers.clear();
}



//*		main functionalities

void Worker::workerLoop() {
	std::vector<ConnectionSocket *>::iterator	it;
	
	std::cout << BOLDGREEN << "\nStarting worker" << RESET << std::endl;
	while (true) {
		try {
			_io_multiplexing_using_epoll();
		}
		catch (const SystemCallException& e) {
			std::cout << std::endl << BOLDRED "Exception >>" << e.what() << RESET << std::endl;
			continue ;
		}
		_handle_new_connectionS();
		_serve_clientS();
	}
}


//*		private helper functions

void	Worker::_io_multiplexing_using_epoll(){
	memset(this->edata.eeventS, 0, sizeof(struct epoll_event) * MAX_EVENTS);
	this->edata.n_events = epoll_wait(this->edata.epoll_fd, this->edata.eeventS, MAX_EVENTS, -1);
	if (-1 == this->edata.n_events) {
		throw SystemCallException("epoll_wait()");
	}
}

void	Worker::_handle_new_connectionS() {
	
	int							cli_socket;
	std::string					client_IP;
	std::string					server_IP;
	const struct epoll_event*	eevent;
	
	try {
		for (
			VectorServ::iterator serv_it = servers.begin();
			serv_it != servers.end();
			serv_it++)
		{
			eevent = this->edata.getEpollEvent((*serv_it).server_fd);
			if ( eevent && eevent->events & EPOLLIN)
			{
				cli_socket = _create_ConnectionSocket((*serv_it), client_IP, server_IP);
				_epoll_register_ConnectionSocket(cli_socket);

				//*		adding to list of open connections
				(*serv_it).open_connections.push_back(
					new ConnectionSocket(cli_socket, client_IP, server_IP, *serv_it, edata)
				);
			}
		}
	}
	catch (const std::exception& e) {
		return ;
	}
}

void	Worker::_serve_clientS( void ) {
	ConnectionSocket*	connection;
	
	for (
		VectorServ::iterator serv_it = servers.begin();
		serv_it != servers.end();
		serv_it++
	)
	{
		for (
			VectorCli::iterator cli_it = (*serv_it).open_connections.begin();
			cli_it != (*serv_it).open_connections.end();
			/*...no increment*/
		)
		{
			connection = (*cli_it);
			try {
				connection->serve_client();
				cli_it++;
			}
			catch (const SockEof& e) {
				std::cout << std::endl << BOLDRED "Exception >>" << e.what() << RESET << std::endl;
				cli_it = (*serv_it).open_connections.erase(cli_it);
				delete connection;
			}
		}
	}
}

/**
 * @brief this function accepts a new connection and sets client and server selected interface IPs.
 * 
 * @param server 
 * @param client_IP 
 * @param server_IP 
 * @return int : the socket file descriptor for the new connection.
 */
int	Worker::_create_ConnectionSocket(
	t_server& server,
	std::string& client_IP, std::string& server_IP
	)
{
	int						cli_socket;
	struct sockaddr_in		cli_addr;
	socklen_t				cli_addr_len 		= sizeof(cli_addr);
	struct sockaddr_in		server_addr;
	socklen_t				server_addr_len		= sizeof(server_addr);

	//*		Accepting
	COUT_DEBUG_INSERTION(std::endl << YELLOW << "_create_ConnectionSocket() called()" << RESET << std::endl);

	cli_socket = accept(server.server_fd, (struct sockaddr *)&cli_addr, &cli_addr_len);
	if (-1 == cli_socket){
		throw (SystemCallException("accept()"));
	}
	std::cout << BOLDGREEN << "\nAccepting new connection" << RESET << std::endl;
	std::cout	<< "----------------  accept()" 
				<< " | ip: " << _getIP()
				<< " | port: " << server.server_port 
				<< " | server_fd: " << server.server_fd 
				<< " | cli_socket: " << cli_socket
				<< std::endl;
	
	//*		Making non-blocking
	_make_socket_non_blocking(cli_socket);

	//*		Setting Remote Client IP
	client_IP = inet_ntoa(cli_addr.sin_addr);
	
	//*		Setting Server selected interface IP
	if (-1 == getsockname(cli_socket, (struct sockaddr*)&server_addr, &server_addr_len))
		throw (SystemCallException("getsockname()"));
	server_IP = inet_ntoa(server_addr.sin_addr);

	return (cli_socket);
}

void	Worker::_epoll_register_ConnectionSocket(int cli_socket) {

	struct epoll_event		cli_epoll_evt_opt;

	cli_epoll_evt_opt.events = EPOLLIN | EPOLLOUT;
	cli_epoll_evt_opt.data.fd = cli_socket;
	if (-1 == epoll_ctl(this->edata.epoll_fd, EPOLL_CTL_ADD, cli_socket, &cli_epoll_evt_opt)) {
		close(cli_socket);
		throw (SystemCallException("epoll_ctl()"));
	}
}


//*		private initialization functions

void	Worker::_init_io_multiplexing() {
	struct epoll_event	eevent;
	
	this->edata.epoll_fd = epoll_create(1);
	if (-1 == this->edata.epoll_fd) {
		throw SystemCallException("epoll_create()");
	}
	eevent.events = EPOLLIN;
	for (VectorServ::iterator it = servers.begin(); it != servers.end(); it++)
	{
		eevent.data.fd = (*it).server_fd;
		if (-1 == epoll_ctl(
			this->edata.epoll_fd,
			EPOLL_CTL_ADD,
			(*it).server_fd,
			&eevent))
		{
			throw SystemCallException("epoll_ctl()");
		}
	}
}

void	Worker::_server_init(const t_conf_block& conf_server_block) {
	
	//*		conf_server_block has a vector of all its virtual servers.
	//*		Each virtual server has some directives shared across all other virtual servers,
	//*		which are server-common directives, + some specific ones for the specific virtual server.
	//*		we only care about the common directives.
	const std::map<std::string, std::string>&	server_directives
		= conf_server_block.sub_blocks[0].directives;
	
	std::cout << BOLDGREEN "Starting new server--------------" << RESET << std::endl;
	
	this->servers.push_back(t_server(conf_server_block));
	this->servers.back().server_port = std::atoi(
		server_directives.at("listen").c_str()
	);
	_create_server_socket();
	_set_socket_as_reusable();
	_init_server_addr(server_directives);
	_bind_server_socket_to_ip();
	_make_server_listening();
	_make_socket_non_blocking(this->servers.back().server_fd);

	_print_server_ip_info();
}

void	Worker::_create_server_socket() {
	std::cout << "----------------  socket()" << std::endl;
	this->servers.back().server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == this->servers.back().server_fd)
		throw SystemCallException("socket()");
}

void	Worker::_set_socket_as_reusable() {
	int reuse = 1;
	if (-1 == setsockopt(this->servers.back().server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse))){
		throw SystemCallException("setsockopt()");
	}
}

void	Worker::_init_server_addr(
	const std::map<std::string, std::string>& server_directives
	)
{
	std::string									host = "ANY";
	in_addr_t									ip;

	this->servers.back().server_addr_len = sizeof(this->servers.back().server_addr);
	std::memset(&this->servers.back().server_addr, 0, this->servers.back().server_addr_len);
	this->servers.back().server_addr.sin_family = AF_INET;
	this->servers.back().server_addr.sin_port = htons(this->servers.back().server_port);
	if (server_directives.end() == server_directives.find("host"))
		ip = INADDR_ANY;
	else {
		if ("localhost" == server_directives.at("host"))
			host = "127.0.0.1";
		else
			host = server_directives.at("host");
		ip = inet_addr(host.c_str());
	}
	COUT_DEBUG_INSERTION(\
		GREEN"new server with ip ===>\tas string : |" << host \
		<< "|; as number : " << ip << RESET << std::endl\
	)
	if ((in_addr_t)(-1) == ip)
		throw (
			std::runtime_error(
				"Worker::_init_server_addr() : wrong IP in \"host\" directive"));
	this->servers.back().server_addr.sin_addr.s_addr = ip;
}

void	Worker::_bind_server_socket_to_ip() {
	std::cout << "----------------  bind()" << std::endl;
	if (-1 == bind(
		this->servers.back().server_fd,
		(struct sockaddr *)&this->servers.back().server_addr, 
		this->servers.back().server_addr_len
		))
		throw SystemCallException("bind()");
}

void	Worker::_make_server_listening() {
	std::cout << "----------------  listen()" << std::endl;
	if (-1 == listen(this->servers.back().server_fd, INT_MAX))
		throw SystemCallException("listen()");
}

void	Worker::_make_socket_non_blocking(int sock_fd) {
	int flags = fcntl(sock_fd, F_GETFL, 0);
	if (-1 == flags)
		throw SystemCallException("fcntl()");
	flags |= O_NONBLOCK;
	if (-1 == fcntl(sock_fd, F_SETFL, flags))
		throw SystemCallException("fcntl()");
}

std::string		Worker::_getIP(){
	char ip[INET_ADDRSTRLEN];

	inet_ntop(AF_INET, &(this->servers.back().server_addr.sin_addr), ip, INET_ADDRSTRLEN);

	return ((static_cast<char*>(ip)));
}

void	Worker::_print_server_ip_info() {

	std::cout << "Local address\t: " << _getIP() << std::endl;
	std::cout << "Local port\t: " << ntohs(this->servers.back().server_addr.sin_port) << std::endl;  
	std::cout << "Server_fd\t: " << this->servers.back().server_fd << std::endl;  
}


//*		Canonical Form Shit

Worker::Worker(const Worker & cpy){
	*this = cpy;
}

Worker& Worker::operator=(const Worker & cpy){
	(void)cpy;
	
	throw (std::exception());
	return (*this);
}
