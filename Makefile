CFLAGS := -Wall -Wextra -g -Iinclude -fPIC

SRCS := buffer.c ipc.c list.c reactor.c ref.c smsg.c util.c
OBJS := $(patsubst %.c,%.o,$(SRCS))

all : chatd chat
chatd : libelib.so chatd.o
chat : libelib.so chat.o

libelib.so : $(OBJS)
	$(CC) -shared -o $@ $^

clean :
	rm -f *.o chatd chat libelib.so
