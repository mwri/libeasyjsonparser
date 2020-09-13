#ifndef EASYJSONPARSER_INCLUDED
#define EASYJSONPARSER_INCLUDED


#define EASYJSONPARSER_SUCCESS                      0x00000000
#define EASYJSONPARSER_ERROR_FILEOPEN               0x00001001
#define EASYJSONPARSER_ERROR_LIBJSONC_PARSE         0x00005002
#define EASYJSONPARSER_ERROR_LIBJSONC_SCAN          0x00005003
#define EASYJSONPARSER_ERROR_PARSE_UNEXPECTED       0x00001004
#define EASYJSONPARSER_ERROR_SCHEMA_NOCHILDREN      0x00002005
#define EASYJSONPARSER_ERROR_SCHEMA_UNEXPECTED_KEY  0x00002006
#define EASYJSONPARSER_ERROR_SCHEMA_MANDATES_STRING 0x00002007
#define EASYJSONPARSER_ERROR_SCHEMA_MANDATES_INT    0x00002008
#define EASYJSONPARSER_ERROR_SCHEMA_MANDATES_MAP    0x00002009
#define EASYJSONPARSER_ERROR_SCHEMA_MANDATES_LIST   0x0000200a
#define EASYJSONPARSER_ERROR_SCHEMA_MANDATES_DOUBLE 0x00002010
#define EASYJSONPARSER_ERROR_SCHEMA_MANDATES_BOOL   0x00002011
#define EASYJSONPARSER_ERROR_SCHEMA_MANDATES_NULL   0x00002012
#define EASYJSONPARSER_ERROR_SCHEMA_INVALID         0x0000200b

#define EASYJSONPARSER_ERROR_FATAL_BITS             0x00001000
#define EASYJSONPARSER_ERROR_SCHEMA_BITS            0x00002000
#define EASYJSONPARSER_ERROR_LIBJSONC_BITS          0x00004000


#define EASYJSONPARSER_LOG_LEVEL_NONE  0x0000
#define EASYJSONPARSER_LOG_LEVEL_ERROR 0x0100
#define EASYJSONPARSER_LOG_LEVEL_TRACE 0x0200


#define EASYJSONPARSER_SCHEMA_END       0x0000
#define EASYJSONPARSER_SCHEMA_INT       0x0001
#define EASYJSONPARSER_SCHEMA_STR       0x0002
#define EASYJSONPARSER_SCHEMA_MAP       0x0004
#define EASYJSONPARSER_SCHEMA_LST       0x0008
#define EASYJSONPARSER_SCHEMA_DBL       0x0010
#define EASYJSONPARSER_SCHEMA_BOO       0x0011
#define EASYJSONPARSER_SCHEMA_NUL       0x0012
#define EASYJSONPARSER_SCHEMA_ROO       0x0100

#define EASYJSONPARSER_SCHEMA_TYPE_BITS 0x00ff


typedef struct easyjsonparser_stack_st easyjsonparser_stack;
typedef struct easyjsonparser_schema_st easyjsonparser_schema;


typedef struct easyjsonparser_stack_st {
  char *           key;
  easyjsonparser_stack * prev;
} easyjsonparser_stack;


typedef struct easyjsonparser_schema_st {
  char * key;
  int    type;
  void * data;
  char * descr;
} easyjsonparser_schema;


extern void   easyjsonparser_set_loglevel (int loglevel);
extern void   easyjsonparser_set_logger (void (*logger)(int, const char *));
extern void   easyjsonparser_set_errhandler (int (*handler)(int, const void *, const char *, const char *));
extern void   easyjsonparser_log (int level, const char *, ...);
extern int    easyjsonparser_parse_file (const char * filename, easyjsonparser_schema * ys, void * cfg);
extern int    easyjsonparser_parse_string (const char * input_string, easyjsonparser_schema * ys, void * cfg);
extern char * easyjsonparser_stack_path (easyjsonparser_stack * stack);


#define EASYJSONPARSER_SCHEMA(name, root_type)   easyjsonparser_schema name[] = { { 0, EASYJSONPARSER_SCHEMA_ROO | root_type, 0, 0 },
#define EASYJSONPARSER_SUBSCHEMA(name)           easyjsonparser_schema name[] = {
#define EASYJSONPARSER_STR(name, handler, descr) { name, EASYJSONPARSER_SCHEMA_STR, handler, descr }
#define EASYJSONPARSER_INT(name, handler, descr) { name, EASYJSONPARSER_SCHEMA_INT, handler, descr }
#define EASYJSONPARSER_MAP(name, child, descr)   { name, EASYJSONPARSER_SCHEMA_MAP, child,   descr }
#define EASYJSONPARSER_LST(name, child, descr)   { name, EASYJSONPARSER_SCHEMA_LST, child,   descr }
#define EASYJSONPARSER_DBL(name, handler, descr) { name, EASYJSONPARSER_SCHEMA_DBL, handler, descr }
#define EASYJSONPARSER_BOO(name, handler, descr) { name, EASYJSONPARSER_SCHEMA_BOO, handler, descr }
#define EASYJSONPARSER_NUL(name, handler, descr) { name, EASYJSONPARSER_SCHEMA_NUL, handler, descr }
#define EASYJSONPARSER_END()                     { 0, 0, 0 } }


#endif // EASYJSONPARSER_INCLUDED
