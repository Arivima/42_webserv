/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EpollData.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: earendil <earendil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/01 17:56:16 by earendil          #+#    #+#             */
/*   Updated: 2023/06/01 19:27:35 by earendil         ###   ########.fr       */
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