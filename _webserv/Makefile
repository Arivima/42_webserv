# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: earendil <earendil@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/05/29 12:15:29 by mmarinel          #+#    #+#              #
#    Updated: 2023/06/18 18:52:29 by earendil         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

all:
	g++ -Wall -Werror -Wextra main.cpp utils.cpp EpollData.cpp Worker.cpp ConnectionSocket.cpp Request.cpp Response.cpp Config.cpp -o mini_serv

debug:
	g++ -DDEBUG=1 -Wall -Werror -Wextra main.cpp utils.cpp EpollData.cpp Worker.cpp ConnectionSocket.cpp Request.cpp Response.cpp Config.cpp -o mini_serv

clean:
	rm -rf mini_serv