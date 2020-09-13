/// \file
/// \brief More advanced 'hello universe' example for libeasyjson.
///
/// This example builds on 'ey_hello_world' and demonstrates how to handle
/// unfixed keys (i.e. a key that can be anything) and lists, and shows
/// how to adjust the log level replace the logger with an alternative
/// callback.
///
/// This time the JSON file to be parsed will look like this:
///
/// {
///   "restapi": {
///     "base-path": "/api",
///     "port": 80,
///     "ssl": false
///   },
///   "users": {
///     "john": {
///       "access": [
///         "read"
///       ],
///       "password": "asdfgh"
///     },
///     "michael": {
///       "access": [
///         "admin"
///       ],
///       "password": "qwerty"
///     },
///     "molly": {
///       "access": [
///         "write",
///         "read"
///       ],
///       "password": "zxcvb"
///     }
///   },
///   "version": "1.5.4"
/// }
///
/// Here users have been added, each one having a password and a list of
/// groups. Of course usually one would use a database for this but it
/// makes a good contrived example!


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <stdarg.h>

#include "easyjsonparser.h"


/// The structures the JSON file will be parsed into.

typedef struct {
  char * username;
  char * password;
  int    read;
  int    write;
  int    admin;
} hello_user;

typedef struct {
  int        valid;
  char *     version;
  int        svr_port;
  int        svr_ssl;
  char *     svr_base_path;
  hello_user users[10];
} hello_config;


/// Declare callbacks.

static void ey_handle_version (easyjsonparser_stack * stack, char * val, hello_config * cfg);
static void ey_handle_svr_port (easyjsonparser_stack * stack, int val, hello_config * cfg);
static void ey_handle_svr_ssl (easyjsonparser_stack * stack, int val, hello_config * cfg);
static void ey_handle_svr_base_path (easyjsonparser_stack * stack, char * val, hello_config * cfg);
static void ey_handle_user_password (easyjsonparser_stack * stack, char * val, hello_config * cfg);
static void ey_handle_user_access (easyjsonparser_stack * stack, char * val, hello_config * cfg);


/// JSON schema.

static easyjsonparser_schema * schema ()
{
  static EASYJSONPARSER_SUBSCHEMA(server_js)
    EASYJSONPARSER_INT("port",      &ey_handle_svr_port,      "TCP port"     ),
    EASYJSONPARSER_BOO("ssl",       &ey_handle_svr_ssl,       "SSL enabled"  ),
    EASYJSONPARSER_STR("base-path", &ey_handle_svr_base_path, "URL base path"),
    EASYJSONPARSER_END();

  static EASYJSONPARSER_SUBSCHEMA(user_access_js)
    EASYJSONPARSER_STR(NULL, &ey_handle_user_access, "User access privileges"),
    EASYJSONPARSER_END();

  static EASYJSONPARSER_SUBSCHEMA(user_js)
    EASYJSONPARSER_STR("password", &ey_handle_user_password, "Password"              ),
    EASYJSONPARSER_LST("access",   user_access_js,           "User access privileges"),
    EASYJSONPARSER_END();

  static EASYJSONPARSER_SUBSCHEMA(users_js)
    EASYJSONPARSER_MAP(NULL, user_js, "User"),
    EASYJSONPARSER_END();

  static EASYJSONPARSER_SCHEMA(js, EASYJSONPARSER_SCHEMA_MAP)
    EASYJSONPARSER_STR("version", &ey_handle_version, "Configuration version"),
    EASYJSONPARSER_MAP("restapi", server_js,          "RESTful API server"   ),
    EASYJSONPARSER_MAP("users",   users_js,           "Users"                ),
    EASYJSONPARSER_END();

  return js;
}


/// Schema parsing callbacks.

static void ey_handle_version (easyjsonparser_stack * stack, char * val, hello_config * cfg)
{
  cfg->version = (char *) malloc(strlen(val) + 1);
  strcpy(cfg->version, val);
}

static void ey_handle_svr_port (easyjsonparser_stack * stack, int val, hello_config * cfg)
{
  cfg->svr_port = val;
}

static void ey_handle_svr_ssl (easyjsonparser_stack * stack, int val, hello_config * cfg)
{
  cfg->svr_ssl = val;
}

static void ey_handle_svr_base_path (easyjsonparser_stack * stack, char * val, hello_config * cfg)
{
  cfg->svr_base_path = (char *) malloc(strlen(val) + 1);
  strcpy(cfg->svr_base_path, val);
}

static void ey_handle_user_password (easyjsonparser_stack * stack, char * val, hello_config * cfg)
{
  for (int i = 0; i < 10; i++) {
    if (cfg->users[i].username == NULL) {
      cfg->users[i].username = (char *) malloc(strlen(stack->prev->key) + 1);
      strcpy(cfg->users[i].username, stack->prev->key);
    }
    if (strcmp(cfg->users[i].username, stack->prev->key) == 0) {
      cfg->users[i].password = (char *) malloc(strlen(val) + 1);
      strcpy(cfg->users[i].password, val);
      return;
    }
  }
  fprintf(stderr, "ey_hello_universe: reading configuration, run out of space for new user \"%s\"\n", stack->prev->key);
  cfg->valid = 0;
}

static void ey_handle_user_access (easyjsonparser_stack * stack, char * val, hello_config * cfg)
{
  for (int i = 0; i < 10; i++) {
    if (cfg->users[i].username == NULL) {
      cfg->users[i].username = (char *) malloc(strlen(stack->prev->key) + 1);
      strcpy(cfg->users[i].username, stack->prev->key);
    }
    if (strcmp(cfg->users[i].username, stack->prev->key) == 0) {
      if (strcmp(val, "admin") == 0) {
        cfg->users[i].admin = 1;
        cfg->users[i].read = 1;
        cfg->users[i].write = 1;
      } else if (strcmp(val, "read") == 0) {
        cfg->users[i].read = 1;
      } else if (strcmp(val, "write") == 0) {
        cfg->users[i].write = 1;
      } else {
        fprintf(stderr, "ey_hello_universe: reading configuration, ignoring unrecognised access \"%s\" for user \"%s\"\n", val, stack->prev->key);
      }
      return;
    }
  }
  fprintf(stderr, "ey_hello_universe: reading configuration, run out of space for new user \"%s\"\n", stack->prev->key);
  cfg->valid = 0;
}


/// Custom logger function for libeasyjsonparser

static void ey_logger  (int level, const char * msg)
{
  char * level_str = level == EASYJSONPARSER_LOG_LEVEL_ERROR
    ? "error"
    : "trace";

  // if (level > EASYJSONPARSER_LOG_LEVEL_ERROR)
  //   return;

  fprintf(stderr, "ey_hello_universe: %s: libeasyjsonparser: %s\n", level_str, msg);

  return;
}


/// Parse the file and return the resultant configuration structure.

int main (int argc, char ** argv)
{
  easyjsonparser_set_logger(ey_logger);

  hello_config * cfg = malloc(sizeof(hello_config));
  for (int i = 0; i < 10; i++) {
    cfg->users[i].username = NULL;
    cfg->users[i].password = NULL;
    cfg->users[i].read     = 0;
    cfg->users[i].write    = 0;
    cfg->users[i].admin    = 0;
  }

  cfg->valid = 1;

  // For unit tests.
  char * filename = argv[1];
  if (filename == NULL) {
    filename = (char *) malloc(strlen(argv[0]) + strlen(".json") + 1);
    sprintf(filename, "%s.json", argv[0]);
    printf("%s\n", filename);
  }

  if (easyjsonparser_parse_file(filename, schema(), cfg) != EASYJSONPARSER_SUCCESS || !cfg->valid) {
    fprintf(stderr, "ey_hello_universe: reading configuration failed, sorry\n");
    exit(1);
  }

  printf("ey_hello_universe: successfully parsed config (version %s)\n",
         cfg->version);
  printf("ey_hello_universe: REST API (SSL=%s, port=%d, URL=%s)\n",
         (cfg->svr_ssl ? "true" : "false"), cfg->svr_port, cfg->svr_base_path);
  printf("ey_hello_universe:     Users:\n");
  for (int i = 0; i < 10 && cfg->users[i].username != NULL; i++) {
    printf("ey_hello_universe:         %s (password %s)\n", cfg->users[i].username, cfg->users[i].password);
    printf("ey_hello_universe:             admin = %s\n", cfg->users[i].admin ? "YES" : "NO");
    printf("ey_hello_universe:              read = %s\n", cfg->users[i].read ? "YES" : "NO");
    printf("ey_hello_universe:             write = %s\n", cfg->users[i].write ? "YES" : "NO");
  }

  exit(0);
}
