/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EpollData.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmarinel <mmarinel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/01 18:15:06 by earendil          #+#    #+#             */
/*   Updated: 2023/07/13 18:16:32 by mmarinel         ###   ########.fr       */
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
