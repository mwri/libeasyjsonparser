all: ejp_hello_tiny ejp_hello_world ejp_hello_universe

ejp_hello_tiny: Makefile ejp_hello_tiny.c
	gcc ejp_hello_tiny.c -Wall -g -O0 -I../src -L../src/.libs -Wl,-rpath ../src/.libs -o ejp_hello_tiny -leasyjsonparser

ejp_hello_world: Makefile ejp_hello_world.c
	gcc ejp_hello_world.c -Wall -g -O0 -I../src -L../src/.libs -Wl,-rpath ../src/.libs -o ejp_hello_world -leasyjsonparser

ejp_hello_universe: Makefile ejp_hello_universe.c
	gcc ejp_hello_universe.c -Wall -g -O0 -I../src -L../src/.libs -Wl,-rpath ../src/.libs -o ejp_hello_universe -leasyjsonparser

clean:
	rm -f ejp_hello_tiny ejp_hello_world ejp_hello_universe

check:

install:
