# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mmarinel <mmarinel@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/05/29 12:15:29 by mmarinel          #+#    #+#              #
#    Updated: 2023/07/14 17:57:34 by mmarinel         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Makefile
# 	create executable "Webserv"
# 	create testing folder in "/var/www", will all necessary material for Postman tests
#	debug rule -> output all debug info

NAME 					:= Webserv
# Sources
SRCS_DIR				:= ./src
SRCS					:= $(shell find $(SRCS_DIR) -name "*.cpp" -print)
# Objects 
OBJS_DIR				:= ./obj
OBJS					:= $(patsubst $(SRCS_DIR)%,$(OBJS_DIR)%,$(SRCS:.cpp=.o))
# Headers 
INCLUDE_DIR				:= ./include
INCLUDES				:= $(shell find $(INCLUDE_DIR) -name "*.hpp" -print)
# Compilation & Flags
CC						:= c++
FLAGS 					:= -Wall -Wextra -Werror -std=c++98 -Wshadow -I $(INCLUDE_DIR)
FLAG_DEBUG				:= -DDEBUG=1 -Wall -Wextra -Werror -std=c++98 -Wshadow -I $(INCLUDE_DIR) 
# Testing material
TESTING_ROOT			:= ./var/www/test
TESTING_BACKUP			:= ../back-up_postman_folders

# Rules
all:					$(NAME) $(SET_UP_TESTING)

debug:					$(SET_UP_TESTING)
						FLAGS=FLAG_DEBUG
						@$(MAKE) $(NAME)
						
$(NAME):				$(OBJS)
						@$(CC)  $(FLAGS) $(OBJS) -o $(NAME)
						@echo $(GREEN)$(NAME)"\tSuccessfully compiled" $(RESET)

$(OBJS_DIR)/%.o: 		$(SRCS_DIR)/%.cpp $(INCLUDES)
						# @mkdir -p $(@D)
						# @$(CC) -c $(FLAGS) -DCOMPILE_MODE=$(COMPILE_MODE_VAL) $< -o $@

$(OBJS): 				| $(OBJS_DIR)

$(OBJS_DIR):
						@mkdir $(OBJS_DIR)

$(SET_UP_TESTING):			
						@cp -r $(TESTING_BACKUP) $(TESTING_ROOT)

						@chmod a-rwx	"$(TESTING_ROOT)/test_delete/1_subdir_nok/2/3_not_authorized"
						@chmod a-rwx 	"$(TESTING_ROOT)/test_delete/1_subfile_nok/2/not-authorized-file.txt"
						./$(TESTING_ROOT)/test_delete/1_subfile_busy/2/nyancat & PID_NYANCAT_1=$$!

						@chmod a-rwx 	"$(TESTING_ROOT)/test_delete_redirection/1_subdir_nok/2/3_not_authorized"
						@chmod a-rwx 	"$(TESTING_ROOT)/test_delete_redirection/1_subfile_nok/2/not-authorized-file.txt"
						./$(TESTING_ROOT)/test_delete_redirection/1_subfile_busy/2/nyancat & PID_NYANCAT_2=$$!

						@chmod a-rwx	"$(TESTING_ROOT)/test_get/not-authorized-file.txt"
						./$(TESTING_ROOT)/test_get/nyancat & PID_NYANCAT_3=$$!
						
						@chmod a-rwx 	"$(TESTING_ROOT)/test_not_authorized"

						@echo $(GREEN)$(NAME)"\tSuccessfully set-up the testing material" $(RESET)

$(CLEAN_UP_TESTING):
						@chmod a+rwx	"$(TESTING_ROOT)/test_delete/1_subdir_nok/2/3_not_authorized"
						@chmod a+rwx 	"$(TESTING_ROOT)/test_delete/1_subfile_nok/2/not-authorized-file.txt"
						@kill $$PID_NYANCAT_1

						@chmod a+rwx 	"$(TESTING_ROOT)/test_delete_redirection/1_subdir_nok/2/3_not_authorized"
						@chmod a+rwx 	"$(TESTING_ROOT)/test_delete_redirection/1_subfile_nok/2/not-authorized-file.txt"
						@kill $$PID_NYANCAT_2

						@chmod a+rwx	"$(TESTING_ROOT)/test_get/not-authorized-file.txt"
						@kill $$PID_NYANCAT_3
						
						@chmod a+rwx 	"$(TESTING_ROOT)/test_not_authorized"					

						@rm -rf $(TESTING_ROOT)
						@echo $(GREEN)$(NAME)"\tSuccessfully cleaned up the testing material" $(RESET)

clean:				
						@rm -rf $(OBJS_DIR)
						@echo $(GREEN)"\t\tSuccessfully removed objects" $(RESET)

fclean: 				clean $(CLEAN_UP_TESTING)
						@rm -rf $(NAME)
						@echo $(GREEN)"\t\tSuccessfully removed executable and testing folder" $(RESET)

re: 					fclean all

.PHONY: 				all clean fclean re

# Colors
RESET					:= "\033[0m"
BLACK					:= "\033[30m"
RED						:= "\033[31m"
GREEN					:= "\033[32m"
YELLOW					:= "\033[33m"
BLUE					:= "\033[34m"
MAGENTA					:= "\033[35m"
CYAN					:= "\033[36m"
WHITE					:= "\033[37m"
BOLDBLACK				:= "\033[1m\033[30m"
BOLDRED					:= "\033[1m\033[31m"
BOLDGREEN				:= "\033[1m\033[32m"
BOLDYELLOW				:= "\033[1m\033[33m"
BOLDBLUE				:= "\033[1m\033[34m"
BOLDMAGENTA				:= "\033[1m\033[35m"
BOLDCYAN				:= "\033[1m\033[36m"
BOLDWHITE				:= "\033[1m\033[37m"

# END Makefile