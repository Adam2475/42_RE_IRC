NAME = ircserv
CC = c++

SRCS = ./src/main.cpp ./src/Server.cpp ./src/User.cpp ./src/Channel.cpp ./src/Commands.cpp \
		./src/utils.cpp ./src/command_utils.cpp

CFLAGS = -std=c++98 -Wall -Wextra -Werror -g

OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	@echo "Linking..."
	@$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "Cleaning object files..."
	@rm -f $(OBJS)

fclean: clean
	@echo "Cleaning executable..."
	@rm -f $(NAME)

re: fclean all

.PHONY: all re clean fclean