CFLAGS := -Wall -Wextra -g -Iinclude -fPIC

SRCS := buffer.c ipc.c list.c reactor.c ref.c smsg.c
OBJS := $(patsubst %.c,%.o,$(SRCS))

all : testd test
testd : libelib.so testd.o
test : libelib.so test.o

libelib.so : $(OBJS)
	$(CC) -shared -o $@ $^

clean :
	rm -f *.o test testd libelib.so
