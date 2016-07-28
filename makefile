P=mbcl
CFLAGS=-g -Wall -std=gnu99 -O3 `pkg-config --cflags libmodbus`
LDLIBS=-LLIBDIR `pkg-config --libs libmodbus`
objects=mbcl.o
$(P): $(objects)

clean:
	rm -rf *.o $(P)
