CFLAGS := -Wall -Wextra -g -Iinclude -fPIC

SRCS := buffer.c ipc.c linesocket.c list.c map.c reactor.c ref.c smsg.c util.c
OBJS := $(patsubst %.c,%.o,$(SRCS))

all : chatd chat
chatd : libelib.so chatd.o
chat : libelib.so chat.o

libelib.so : $(OBJS)
	$(CC) -shared -o $@ $^

clean :
	rm -f *.o chatd chat libelib.so

install :
	mkdir -p /usr/local/include/elib
	cp -r include/elib/* /usr/local/include/elib
	install libelib.so /usr/local/lib
