LIBS=-lulfius -lyder -lorcania -lSDL2 -lSDL2_image -ljansson

all: timer

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

timer: main.o simple_timer.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f *.o timer

