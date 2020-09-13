TESTS = check_easyjsonparser check_hello_tiny check_hello_world check_hello_universe
check_PROGRAMS = $(TESTS)

clean-local:
	rm -f *.gcda *.gcno *.gcov

check_easyjsonparser_SOURCES = check_easyjsonparser.c \
	easyjsonparser_check.c \
	../src/easyjsonparser.c
check_easyjsonparser_CFLAGS = @CHECK_CFLAGS@ -I../src --coverage
check_easyjsonparser_LDFLAGS = -ljson-c
check_easyjsonparser_LDADD = @CHECK_LIBS@

check_hello_tiny_SOURCES = ./../examples/ejp_hello_tiny.c \
	../src/easyjsonparser.c
check_hello_tiny_CFLAGS = @CHECK_CFLAGS@ -I../src
check_hello_tiny_LDFLAGS = -ljson-c
check_hello_tiny_LDADD = @CHECK_LIBS@

check_hello_world_SOURCES = ./../examples/ejp_hello_world.c \
	../src/easyjsonparser.c
check_hello_world_CFLAGS = @CHECK_CFLAGS@ -I../src
check_hello_world_LDFLAGS = -ljson-c
check_hello_world_LDADD = @CHECK_LIBS@

check_hello_universe_SOURCES = ./../examples/ejp_hello_universe.c \
	../src/easyjsonparser.c
check_hello_universe_CFLAGS = @CHECK_CFLAGS@ -I../src
check_hello_universe_LDFLAGS = -ljson-c
check_hello_universe_LDADD = @CHECK_LIBS@
