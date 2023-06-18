# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: earendil <earendil@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/05/29 12:15:29 by mmarinel          #+#    #+#              #
#    Updated: 2023/06/18 11:50:46 by earendil         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

all:
	g++ -Wall -Werror -Wextra main.cpp utils.cpp Config.cpp -o mini_serv

debug:
	g++ -DDEBUG=1 -Wall -Werror -Wextra main.cpp utils.cpp Config.cpp -o mini_serv

clean:
	rm -rf mini_serv