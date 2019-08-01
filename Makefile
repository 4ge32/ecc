PROG=ecc
TEST=test.sh
CC=riscv64-unknown-linux-gnu-gcc
#CC=gcc
OBJDUMP=riscv64-unknown-linux-gnu-objdump
CFLAGS=-Wall -std=c11 -g
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

$(PROG): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

$(OBJS): ecc.h

dump:
	$(OBJDUMP) -d $(PROG)

send:
	scp -P 10000 $(PROG) root@localhost:~

test_send:
	scp -P 10000 $(TEST) root@localhost:~

test: ecc
	./test.sh

clean:
	rm -f ecc *.o *~ tmp*
