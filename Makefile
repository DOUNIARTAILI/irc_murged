NAME = ircserv

SRC		= Channel.cpp Commands.cpp file.cpp main.cpp server.cpp

CC 		= c++

FLAGS	= -Wall -Wextra -Werror -std=c++98 -lcurl #-fsanitize=address -g 

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