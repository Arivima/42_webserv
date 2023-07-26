# ************************************************************************** #
#                                                                            #
#                                                        :::      ::::::::   #
#   ConnectionSocket.cpp                               :+:      :+:    :+:   #
#   By: team_PiouPiou                                +:+ +:+         +:+     #
#       avilla-m <avilla-m@student.42.fr>          +#+  +:+       +#+        #
#       mmarinel <mmarinel@student.42.fr>        +#+#+#+#+#+   +#+           #
#                                                     #+#    #+#             #
#                                                    ###   ########.fr       #
#                                                                            #
# ************************************************************************** #

# Makefile
# 	create executable "Webserv"
# 	create testing folder in "/var/www", will all necessary material for Postman tests
#	debug rule -> output all debug info

NAME 					:= webserv
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
FLAGS_DEFAULT			:= -Wall -Wextra -Werror -std=c++98
FLAG_DEBUG				:= -DDEBUG=1
FLAG_FULLDEBUG			:= -DFULL_DEBUG=1
# Testing material
TESTING_ROOT			:= ./var/www/
TESTING_BACKUP			:= ./testing/Postman/folders_testing

# Rules
all:					$(NAME)
						
debug:					
						@$(MAKE) $(NAME) ADDITIONAL_FLAGS=$(FLAG_DEBUG)

full_debug:					
						@$(MAKE) $(NAME) ADDITIONAL_FLAGS=$(FLAG_FULLDEBUG)

run:					
						@$(MAKE) $(NAME)
						@reset && ./$(NAME)

help:					
						@echo $(BOLDCYAN)$(NAME)"\tlist of available makefile rules\n" $(RESET)
						@echo $(CYAN)"run :\t\tcompiles executable & execute with default configuration file" $(RESET)
						@echo $(CYAN)"debug :\t\tprints request/response info" $(RESET)
						@echo $(CYAN)"full_debug :\tprints full details" $(RESET)
						@echo $(CYAN)"clean :\t\tcleans objects and testing material" $(RESET)
						@echo $(CYAN)"fclean :\tcleans executable" $(RESET)


$(NAME):				$(OBJS)
						@$(MAKE) --silent .SET_UP_TESTING
						@echo $(CYAN)$(NAME)"\tMaking executable" $(RESET)
						@$(CC) $(FLAGS_DEFAULT) $(OBJS) -o $(NAME)
						@echo $(GREEN)$(NAME)"\tSuccessfully compiled" $(RESET)

$(OBJS_DIR)/%.o: 		$(SRCS_DIR)/%.cpp $(INCLUDES)
						@mkdir -p $(@D)
						@$(CC) -c $(FLAGS_DEFAULT) $(ADDITIONAL_FLAGS) -I $(INCLUDE_DIR) $< -o $@

.SET_UP_TESTING:		
						@echo $(CYAN)$(NAME)"\tSetting-up the testing material" $(RESET)
						-cp -r $(TESTING_BACKUP)/* $(TESTING_ROOT) 2>/dev/null
						-chmod a-rwx	"$(TESTING_ROOT)/test_delete/not-authorized-file.txt" 2>/dev/null
						-chmod a-rwx	"$(TESTING_ROOT)/test_delete/1_subdir_nok/2/3_not_authorized" 2>/dev/null
						-chmod a-rwx	"$(TESTING_ROOT)/test_delete/1_subfile_nok/2/not-authorized-file.txt" 2>/dev/null
						-chmod a-rwx	"$(TESTING_ROOT)/test_delete_redirection/not-authorized-file.txt" 2>/dev/null
						-chmod a-rwx	"$(TESTING_ROOT)/test_delete_redirection/1_subdir_nok/2/3_not_authorized" 2>/dev/null
						-chmod a-rwx	"$(TESTING_ROOT)/test_delete_redirection/1_subfile_nok/2/not-authorized-file.txt" 2>/dev/null
						-chmod a-rwx	"$(TESTING_ROOT)/test_get/not-authorized-file.txt" 2>/dev/null
						-chmod a-rwx	"$(TESTING_ROOT)/test_post/notauthorized" 2>/dev/null
						-chmod a-rwx	"$(TESTING_ROOT)/test_notauthorized" 2>/dev/null
						@echo $(GREEN)$(NAME)"\tSuccessfully set-up the testing material" $(RESET)

.CLEAN_UP_TESTING:
						@echo $(CYAN)$(NAME)"\tCleaning up the testing material" $(RESET)
						-chmod a+rwx	"$(TESTING_ROOT)/test_delete/not-authorized-file.txt" 2>/dev/null
						-chmod a+rwx	"$(TESTING_ROOT)/test_delete/1_subdir_nok/2/3_not_authorized" 2>/dev/null
						-chmod a+rwx	"$(TESTING_ROOT)/test_delete/1_subfile_nok/2/not-authorized-file.txt" 2>/dev/null
						-chmod a+rwx	"$(TESTING_ROOT)/test_delete_redirection/not-authorized-file.txt" 2>/dev/null
						-chmod a+rwx	"$(TESTING_ROOT)/test_delete_redirection/1_subdir_nok/2/3_not_authorized" 2>/dev/null
						-chmod a+rwx	"$(TESTING_ROOT)/test_delete_redirection/1_subfile_nok/2/not-authorized-file.txt" 2>/dev/null
						-chmod a+rwx	"$(TESTING_ROOT)/test_get/not-authorized-file.txt" 2>/dev/null
						-chmod a+rwx	"$(TESTING_ROOT)/test_post/notauthorized" 2>/dev/null
						-chmod a+rwx	"$(TESTING_ROOT)/test_notauthorized" 2>/dev/null
						-rm -rf $(TESTING_ROOT)/test_* 2>/dev/null
						@echo $(GREEN)$(NAME)"\tSuccessfully cleaned up the testing material" $(RESET)

clean:					
						@$(MAKE) --silent .CLEAN_UP_TESTING
						@rm -rf $(OBJS_DIR) 
						@echo $(GREEN)$(NAME)"\tSuccessfully removed objects" $(RESET)

fclean: 				clean
						@rm -rf $(NAME)
						@echo $(GREEN)$(NAME)"\tSuccessfully removed executable and testing folder" $(RESET)

re: 					fclean all

.PHONY: 				all clean fclean re .SET_UP_TESTING

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