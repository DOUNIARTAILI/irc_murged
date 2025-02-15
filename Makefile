# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: drtaili <drtaili@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/03/17 23:29:21 by drtaili           #+#    #+#              #
#    Updated: 2024/03/17 23:29:24 by drtaili          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = ircserv

SRC		= Channel.cpp Commands.cpp file.cpp main.cpp server.cpp

CC 		= c++

FLAGS	= -Wall -Wextra -Werror -std=c++98 -lcurl

OBJS 	= $(SRC:.c=.o)

$(NAME): $(OBJS) Channel.hpp Commands.hpp server.hpp
	$(CC) $(FLAGS) $(SRC) -o $(NAME)

all: $(NAME) $(OBJS)

%.o: %.c  Channel.hpp Commands.hpp server.hpp
		$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -rf $(NAME)

fclean: clean

re: fclean all