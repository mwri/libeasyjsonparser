lib_LTLIBRARIES = libeasyjsonparser.la

libeasyjsonparser_la_SOURCES = easyjsonparser.c
libeasyjsonparser_la_LDFLAGS = -export-symbols exports.sym -version-info 0:0:0
libeasyjsonparser_la_LIBADD = -ljson-c
libeasyjsonparser_la_CFLAGS = -Wall

include_HEADERS = easyjsonparser.h

CLEANFILES = *.gcda *.gcno
