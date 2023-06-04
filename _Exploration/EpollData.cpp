/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EpollData.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/01 18:15:06 by earendil          #+#    #+#             */
/*   Updated: 2023/06/02 12:54:28 by earendil         ###   ########.fr       */
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
