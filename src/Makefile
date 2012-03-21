CFLAGS := -Wall -Wextra -g -I../include -fPIC

SRCS := buffer.c linesocket.c list.c map.c reactor.c ref.c util.c
OBJS := $(patsubst %.c,%.o,$(SRCS))

all : libelib.so

libelib.so : $(OBJS)
	$(CC) -shared -o $@ $^

clean :
	rm -f *.o libelib.so

install :
	mkdir -p /usr/local/include/elib
	cp -r include/elib/* /usr/local/include/elib
	install libelib.so /usr/local/lib
