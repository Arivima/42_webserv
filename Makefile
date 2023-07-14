# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mmarinel <mmarinel@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/05/29 12:15:29 by mmarinel          #+#    #+#              #
#    Updated: 2023/07/14 14:16:55 by mmarinel         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #


INCLUDE_DIR="include/"

FLAGS=-Wall -Werror -Wextra -I $(INCLUDE_DIR)

FILES	= 	src/main.cpp \
			src/utils.cpp \
			src/EpollData.cpp \
			src/Worker.cpp \
			src/ConnectionSocket.cpp \
			src/Request.cpp \
			src/Response.cpp \
			src/CGI.cpp \
			src/Exceptions.cpp \
			src/Config.cpp

all:
	g++ $(FLAGS) $(FILES) -o mini_serv

debug:
	g++ -DDEBUG=1 $(FLAGS) $(FILES) -o mini_serv

clean:
	rm -rf mini_serv
