/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EpollData.cpp                                      :+:      :+:    :+:   */
/*   By: team_PiouPiou                                +:+ +:+         +:+     */
/*       avilla-m <avilla-m@student.42.fr>          +#+  +:+       +#+        */
/*       mmarinel <mmarinel@student.42.fr>        +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*                                                    ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "EpollData.hpp"

const struct epoll_event* t_epoll_data::getEpollEvent(int sock_fd) const
{
	int	i;
	
	for (i = 0; i < n_events; i++)
		if (sock_fd == eeventS[i].data.fd)
			break ;
	return (i == n_events ? NULL : &eeventS[i]);
}
