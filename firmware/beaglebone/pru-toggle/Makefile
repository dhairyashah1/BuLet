CFLAGS+=-Wall -Werror
LDLIBS+= -lpthread -lprussdrv -lm

all: example.bin example

clean:
	rm -f example fsk *.o *.bin

example.bin: example.p
	pasm -b $^

carrier.bin: carrier.p
	pasm -b $^

carrier: carrier.o

example: example.o

fsk: fsk.c fsk.p
	pasm -b fsk.p
	$(CC) $(CFLAGS) $(LDLIBS) -o fsk fsk.c
