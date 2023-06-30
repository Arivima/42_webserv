# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mmarinel <mmarinel@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/05/29 12:15:29 by mmarinel          #+#    #+#              #
#    Updated: 2023/06/30 21:36:54 by mmarinel         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #


FILES	= 	main.cpp \
			utils.cpp \
			EpollData.cpp \
			Worker.cpp \
			ConnectionSocket.cpp \
			Request.cpp \
			Response.cpp \
			CGI.cpp \
			Config.cpp

all:
	g++ -Wall -Werror -Wextra $(FILES) -o mini_serv

debug:
	g++ -DDEBUG=1 -Wall -Werror -Wextra $(FILES) -o mini_serv

clean:
	rm -rf mini_serv
