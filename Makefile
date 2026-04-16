# Options
COMPILER = gcc
FLAGS    = -Wall -Wextra -Werror -g3

# Server
SERVER   = server
SRVSRC   = CsocketServer.c
SRVOBJ   = $(SRVSRC:.c=.o)

# Client
CLIENT   = client
CLISRC   = CsocketClient.c
CLIOBJ   = $(CLISRC:.c=.o)

# Server Client
NAME     = socket
SRC      = $(SRVSRC) $(CLISRC)
OBJ      = $(SRVOBJ) $(CLIOBJ)

# Rules:
$(SERVER): $(SRVOBJ)
	$(COMPILER) $(FLAGS) $< -o $(SERVER)

$(CLIENT): $(CLIOBJ)
	$(COMPILER) $(FLAGS) $< -o $(CLIENT)

all: $(NAME)

$(NAME) : $(CLIENT) $(SERVER)

%.o: %.c
	$(COMPILER) $(FLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(CLIENT) $(SERVER)

re: fclean $(NAME)


# Special Targets:

.PHONY: clean

.SECONDARY: $(OBJ)
