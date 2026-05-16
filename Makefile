# Options
COMPILER = c++
FLAGS    = -Wall -Wextra -Werror -g3 -std=c++98
OBJ_dir  = objs/
SRC_dir  = srcs/

# Server
NAME  = ircserv
SRC   = $(SRC_dir)Channel.cpp $(SRC_dir)Client.cpp $(SRC_dir)Dispatcher.cpp \
        $(SRC_dir)DispatcherUtils.cpp $(SRC_dir)Parser.cpp $(SRC_dir)Server.cpp \
        $(SRC_dir)Utils.cpp ircServ.cpp

OBJ   = $(SRC:$(SRC_dir)%.cpp=$(OBJ_dir)%.o)

# Rules
all: $(NAME)

$(NAME): $(OBJ)
	$(COMPILER) $(FLAGS) $^ -o $(NAME)

$(OBJ_dir)%.o: $(SRC_dir)%.cpp | $(OBJ_dir)
	$(COMPILER) $(FLAGS) -c $< -o $@

$(OBJ_dir):
	mkdir -p $(OBJ_dir)

clean:
	rm -rf $(OBJ_dir)

fclean: clean
	rm -f $(NAME)

re: fclean all

# Special Targets:
.PHONY: clean
.SECONDARY: $(OBJ)
