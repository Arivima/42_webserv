/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Worker.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/06 21:15:29 by earendil          #+#    #+#             */
/*   Updated: 2023/06/09 16:58:07 by earendil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Worker.hpp"

//*		main constructors and destructor

Worker::Worker(const t_conf_root_block& conf_enclosing_block) : servers(), edata()
{
	for (size_t i = 0; i < conf_enclosing_block.http.servers.size(); i++)
		this->_server_init(conf_enclosing_block.http.servers[i]);
	_init_io_multiplexing();
}

Worker::~Worker() {
	for (
		VectorServ::iterator serv_it = this->servers.begin();
		serv_it != this->servers.end();
		serv_it++)
	{
		for (
			VectorCli::iterator cli_it = (*serv_it).requests.begin();
			cli_it != (*serv_it).requests.end();
			cli_it++)
		{
			(*serv_it).requests.erase(cli_it);
		}
		(*serv_it).requests.clear();
		std::cout << "Closing server: " << (*serv_it).server_fd << std::endl;
		close((*serv_it).server_fd);
	}
}



//*		main functionalities

void Worker::workerLoop(){
	std::vector<ConnectionSocket *>::iterator	it;
	
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

void	Worker::_serve_clientS( void ) {
	for (
		VectorServ::iterator serv_it = servers.begin();
		serv_it != servers.end();
		serv_it++)
	{
		for (
			VectorCli::iterator cli_it = (*serv_it).requests.begin();
			cli_it != (*serv_it).requests.end();
			)
		{
			try {
				_serve_client(*(*cli_it));
				cli_it++;
			}
			catch (const ConnectionSocket::SockEof& e) {
				std::cout << std::endl << BOLDRED "Exception >>" << e.what() << RESET << std::endl;
				cli_it = (*serv_it).requests.erase(cli_it);
			}
		}
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
	int						cli_socket;
	
	for (
		VectorServ::iterator serv_it = servers.begin();
		serv_it != servers.end();
		serv_it++)
	{
		if (this->edata.getEpollEvent((*serv_it).server_fd))
		{
			cli_socket = _create_ConnectionSocket((*serv_it));
			_epoll_register_ConnectionSocket(cli_socket);
		    //*     modifying rcv buffer
		    // int buffer_size = 8001;
		    // if (-1 == setsockopt(cli_socket, SOL_SOCKET, SO_RCVBUF, &buffer_size, sizeof(buffer_size)))
		    // {
		    //     ft_close(server_fd);
		    //     ftError("setsockopt() failed : setting rcv buffer size");
		    // }
						
			//*     adding to list of clients
			(*serv_it).requests.push_back(new ConnectionSocket(cli_socket, *serv_it, edata));
		}
	}
	// std::cout << std::endl;
}

int	Worker::_create_ConnectionSocket( t_server server ) {
	
	int						cli_socket;
	struct sockaddr_in		cli_addr;
	socklen_t				cli_addr_len = sizeof(cli_addr);

	std::cout  << std::endl << "\033[1m\033[32m""handle_new_connection() called()""\033[0m" << std::endl;
	std::cout << "---------------accept()" << std::endl;
	cli_socket = accept(server.server_fd, (struct sockaddr *)&cli_addr, &cli_addr_len);
	if (-1 == cli_socket){
		throw (SystemCallException("accept()"));
	}
	_make_socket_non_blocking(cli_socket);
	std::cout << "new connection socket : " << cli_socket << std::endl;

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

void	Worker::_serve_client( ConnectionSocket& request ) {
	
	const struct epoll_event*	eevent = edata.getEpollEvent(request.getSockFD());
	
	if (ConnectionSocket::e_READING_REQ == request.getStatus())
	{
		request.parse_line();
	}
	else {
		if (0 == request.flag) {
			if (!eevent || !(eevent->events & EPOLLOUT))
				return ;
	        std::cout << "end of request" << std::endl;
	        request.print_req();
	        std::cout << "response mode" << std::endl;
	        ssize_t bytes_sent = send(request.getSockFD(), SIMPLE_HTML_RESP, SIMPLE_HTML_RESP_SIZE, 0);
	        std::cout << "send done" << std::endl;
			if (bytes_sent > 0){
	        	std::cout << "Response sent to the client.";
	        	std::cout << "| bytes_sent : " << bytes_sent << std::endl;
			}
			else if (-1 == bytes_sent){
	            std::cout << "HERE" << std::endl;
	            close(request.getSockFD());//TODO		forse la metto dentro il distruttore del client
	            // clit = clients.erase(clit);
			}
	        else {
	            std::cout << "Maremma li mortacci" << std::endl;
	        }
	        request.flag = 1;
		}
	}
}



//*		private initialization functions

void	Worker::_server_init(const t_server_block& conf_server_block) {
	this->servers.push_back(t_server(conf_server_block));
	//TODO	use conf block to set server data
	this->servers.back().server_port = DEFAULT_PORT_NUM;
	_create_server_socket();
	_set_socket_as_reusable();
	_init_server_addr();
	_print_server_ip_info();
	_bind_server_socket_to_ip();
	_make_server_listening();
	_make_socket_non_blocking(this->servers.back().server_fd);
}

void	Worker::_create_server_socket() {
	std::cout << "---------------socket()" << std::endl;
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

void	Worker::_init_server_addr() {
	this->servers.back().server_addr_len = sizeof(this->servers.back().server_addr);
	std::memset(&this->servers.back().server_addr, 0, this->servers.back().server_addr_len);
	this->servers.back().server_addr.sin_family = AF_INET;
	this->servers.back().server_addr.sin_port = htons(this->servers.back().server_port);
	this->servers.back().server_addr.sin_addr.s_addr = INADDR_ANY; // bind to any local available address (port)
}

void	Worker::_print_server_ip_info() {
	char ip[INET_ADDRSTRLEN];

	inet_ntop(AF_INET, &(this->servers.back().server_addr.sin_addr), ip, INET_ADDRSTRLEN);
	std::cout << "Local fam\t: " << this->servers.back().server_addr.sin_family << std::endl;
	std::cout << "Local address\t: " << ip << std::endl;
	std::cout << "Local port\t: " << ntohs(this->servers.back().server_addr.sin_port) << std::endl;  
}

void	Worker::_bind_server_socket_to_ip() {
	std::cout << "---------------bind()" << std::endl;
	if (-1 == bind(
		this->servers.back().server_fd, 
		(struct sockaddr *)&this->servers.back().server_addr, 
		this->servers.back().server_addr_len
		))
		throw SystemCallException("bind()");
}

void	Worker::_make_server_listening() {
	std::cout << "---------------listen()" << std::endl;
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

void	Worker::_init_io_multiplexing() {
	struct epoll_event	eevent;
	
	this->edata.epoll_fd = epoll_create(1);
	if (-1 == this->edata.epoll_fd) {
		throw SystemCallException("epoll_create()");
	}
	eevent.events = EPOLLIN;
	eevent.data.fd = this->servers.back().server_fd;
	for (VectorServ::iterator it = servers.begin(); it != servers.end(); it++)
		if (-1 == epoll_ctl(
			this->edata.epoll_fd,
			EPOLL_CTL_ADD,
			(*it).server_fd,
			&eevent))
		{
			throw SystemCallException("epoll_ctl()");
		}
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
