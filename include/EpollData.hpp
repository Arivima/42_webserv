/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EpollData.hpp                                      :+:      :+:    :+:   */
/*   By: team_PiouPiou                                +:+ +:+         +:+     */
/*       avilla-m <avilla-m@student.42.fr>          +#+  +:+       +#+        */
/*       mmarinel <mmarinel@student.42.fr>        +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*                                                    ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EPOLLDATA_HPP
# define EPOLLDATA_HPP

#include <sys/epoll.h>      // epoll
#include <cstdlib>

# define MAX_EVENTS 4096

typedef struct s_epoll_data
{
	int							epoll_fd;
	struct epoll_event			eeventS[MAX_EVENTS];
	int							n_events;

	const struct epoll_event*	getEpollEvent(int sock_fd) const;

}	t_epoll_data;

#endif