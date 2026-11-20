NAME     = woody_woodpacker
CC       = cc
CFLAGS   = -Wall -Wextra -Werror
INCLUDES = -I includes
SRCS     = srcs/main.c srcs/rc4.c srcs/key.c srcs/woody.c srcs/inject.c srcs/elf.c
OBJS     = $(SRCS:.c=.o)

STUB_SRC = srcs/stub.s
STUB_BIN = srcs/stub.bin
STUB_HDR = includes/stub_bin.h

all: $(NAME)

$(NAME): $(STUB_HDR) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

# 1. stub.s -> octets machine bruts
$(STUB_BIN): $(STUB_SRC)
	nasm -f bin $(STUB_SRC) -o $(STUB_BIN)

# 2. octets bruts -> tableau C
$(STUB_HDR): $(STUB_BIN)
	xxd -i -n woody_stub $(STUB_BIN) > $(STUB_HDR)

# les .o dependent du header (pour qu'il soit genere avant)
%.o: %.c $(STUB_HDR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(STUB_BIN) $(STUB_HDR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re